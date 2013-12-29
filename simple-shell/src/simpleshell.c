// SYSTEM INCLUDES
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


// MACRO DEFINITIONS
#define MAX_ARG_SIZE   50
#define TRUE    1
#define FALSE   0
typedef int boolean_t;
typedef char* string_t;


// ERROR CODES
#define ERR_UMASK_PARAM           201
#define ERR_DELIMITER_PARAM       202
#define ERR_WORK_DIR_PARAM        203
#define ERR_ASSERT_PARAM          204
#define ERR_COMMAND_PARAM         205
#define ERR_UNSUPPORTED_PARAM     206

#define ERR_ASSERT_RESOLVE        207
#define ERR_ASSERT_MISMATCH       208
#define ERR_CHDIR_FAILED          209
#define ERR_COMMAND_TOO_MANY_ARGS 210
#define ERR_COMMAND_EXEC_FAILED   211
#define ERR_INVALID_ARG           212
#define ERR_NOT_ENOUGH_MEMORY     213


// GLOBAL DECLARATIONS
extern char **environ;


// LOCAL FUNCTIONS
boolean_t areStringsEqual(const string_t str1, const string_t str2);

boolean_t startsWith(const string_t str1, const string_t str2);

string_t substring(const string_t str, int startPos, int length);

string_t getStringValue(const string_t str );

string_t parseNextStringToken(string_t *tokenStart, const string_t delims );

void printUsage() {
	fprintf( stderr, "Usage ./simpleshell [-u <UMASK>] [-d <DELIMITER>] [-w <WORKING_DIR>] [--assert <ARG1> <ARG2>]  -c <COMMAND_TO_EXEC>\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "  * ReturnCode(%d): Missing <UMASK> after -u\n", ERR_UMASK_PARAM );
	fprintf( stderr, "  * ReturnCode(%d): Missing <DELIMITER> -d\n", ERR_DELIMITER_PARAM );
	fprintf( stderr, "  * ReturnCode(%d): Missing <WORKING_DIRECTORY> after -w\n", ERR_WORK_DIR_PARAM );
	fprintf( stderr, "  * ReturnCode(%d): Missing <ARG1> or <ARG2> after --assert\n", ERR_ASSERT_PARAM );
	fprintf( stderr, "  * ReturnCode(%d): Missing <COMMAND_TO_EXEC> after -c OR missing -c completely\n", ERR_COMMAND_PARAM );
	fprintf( stderr, "  * ReturnCode(%d): Unsupported command line argument\n", ERR_UNSUPPORTED_PARAM );
	fprintf( stderr, "  * ReturnCode(%d): Assert arg value cannot be resolved\n", ERR_ASSERT_RESOLVE );
	fprintf( stderr, "  * ReturnCode(%d): Assert arg value mismatch\n", ERR_ASSERT_MISMATCH );
	fprintf( stderr, "  * ReturnCode(%d): Failed to chdir to working directory\n", ERR_CHDIR_FAILED );
	fprintf( stderr, "  * ReturnCode(%d): Command after -c consists of too many args (maximum is %d)\n", ERR_COMMAND_TOO_MANY_ARGS, MAX_ARG_SIZE );
	fprintf( stderr, "  * ReturnCode(%d): Invalid args in function call)\n", ERR_INVALID_ARG );
	fprintf( stderr, "  * ReturnCode(%d): Not enough memory\n", ERR_NOT_ENOUGH_MEMORY );
}


int main(int argc, char *argv[]){

	boolean_t isUmaskSet = FALSE;
	int umaskValue = 0;

	int isVerbose = FALSE;
	
	boolean_t isWorkingDirectorySet = FALSE;
	string_t workingDirectory;

	boolean_t isCommandSet = FALSE;
	string_t delimiters = " ";
	string_t command;

	boolean_t isAssertSet = FALSE;
	string_t assertArg1;
	string_t assertArg2;

	// process arguments
	int i = 0;
	while ((i+1)<argc) {
		// Move to next argument
		i++;

	        #ifdef TRACE
		  fprintf( stderr, "TRACE: Processing Arg %d / %d = \"%s\"\n", i, argc, argv[i] );
		#endif

		///////
		// Handle Verbose (-v)
		if ( areStringsEqual(argv[i], "-v") == TRUE ) {
		        #ifdef TRACE
			  fprintf( stderr, "TRACE: Verbose flag set \"-v\" at %i\n", i );
			#endif

			isVerbose = TRUE;
			continue;
		}

		///////
		// Handle UMASK (-u)
		if ( areStringsEqual(argv[i], "-u") == TRUE ) {
		        #ifdef TRACE
			  fprintf( stderr, "TRACE: Processing \"-u\" at %i\n", i );
			#endif

			// Check that we have next argument
			i++;			
			if (i >= argc ) {
				fprintf( stderr, "ERROR: Found \"-u\" followed by no arg\n" );
				exit( ERR_UMASK_PARAM );				
			}

			// Parse Umask
			umaskValue = strtol( argv[i], NULL, 8 );
			isUmaskSet = TRUE;

		        #ifdef TRACE
			  fprintf( stderr, "TRACE: Umask=%o\n", umaskValue );
			#endif

			continue;
		}

		///////
		// Handle DELIMITER IN COMMAND (-d)
		if ( areStringsEqual(argv[i], "-d") == TRUE ) {
		        #ifdef TRACE
			  fprintf( stderr, "TRACE: Processing \"-d\" at %i\n", i );
			#endif

			// Check that we have next argument
			i++;			
			if (i >= argc ) {
				fprintf( stderr, "ERROR: Found \"-d\" followed by no arg\n" );
				exit( ERR_COMMAND_PARAM );				
			}

			// Parse Umask
			delimiters = argv[i];

		        #ifdef TRACE
			  fprintf( stderr, "TRACE: Command delimiter=\"%s\"\n", delimiters );
			#endif

			continue;
		}

		///////
		// Handle Working Directory (-w)
		if ( areStringsEqual(argv[i], "-w") == TRUE ) {
		        #ifdef TRACE
			  fprintf( stderr, "TRACE: Processing \"-w\" at %i\n", i );
			#endif

			// Check that we have next argument
			i++;			
			if (i >= argc) {
				fprintf( stderr, "ERROR: Found \"-w\" followed by no arg\n" );
				exit( ERR_WORK_DIR_PARAM );				
			}

			isWorkingDirectorySet = TRUE;
			workingDirectory = argv[i];

			#ifdef TRACE
			  fprintf( stderr, "TRACE: Dir=%s\n", argv[i] );
			#endif

			continue;
		}

		///////
		// Handle "--assert"
		if (  areStringsEqual(argv[i], "--assert") == TRUE  ) {
                        #ifdef TRACE
                          fprintf( stderr, "TRACE: Processing \"--assert\" at %i\n", i );
                        #endif
			
                        // Check that we have next argument
                        i++;
                        if (i >= argc) {
                                fprintf( stderr, "ERROR: Found \"--assert\" followed by no arg\n" );
                                exit( ERR_ASSERT_PARAM );
                        }
			assertArg1 = argv[i];

                        // Check that we have next argument
                        i++;
                        if (i >= argc) {
                                fprintf( stderr, "ERROR: Found \"--assert\" followed by only one arg\n" );
                                exit( ERR_ASSERT_PARAM );
                        }
			assertArg2 = argv[i];
			isAssertSet = TRUE;

	                #ifdef TRACE
                          fprintf( stderr, "TRACE: Assert \"%s\" and \"%s\"\n", assertArg1, assertArg2 );
                        #endif

                        continue;
		}
		
		///////
		// Handle "-c"
		if ( areStringsEqual(argv[i], "-c") == TRUE ) {	
		        #ifdef TRACE
			  fprintf( stderr, "TRACE: Processing \"-c\" at %i\n", i );
			#endif

			// Check that we have next argument
			i++;			
			if (i >= argc) {
				fprintf( stderr, "ERROR: Found \"-c\" followed by no arg\n" );
				exit( ERR_COMMAND_PARAM );				
			}

			isCommandSet = TRUE;
			command = argv[i];

			#ifdef TRACE
			  fprintf( stderr, "TRACE: Command=\"%s\"\n", argv[i] );
			#endif

			continue;
		}

		fprintf( stderr, "ERROR: Unsupported argument %s\n", argv[i] );
		printUsage();
		exit( ERR_UNSUPPORTED_PARAM );
	}


	/////
	// Check that -c was set
	if (isCommandSet == FALSE) {
		fprintf( stderr, "ERROR: Command not specified - \"-c\" option missing !\n" );
		printUsage();
		exit( ERR_COMMAND_PARAM );
	}

	/////
	// Compare test
	if (isAssertSet == TRUE) {
		string_t arg1Value = getStringValue(assertArg1);
		string_t arg2Value = getStringValue(assertArg2);
		if (arg1Value == NULL || arg2Value == NULL) {
		  fprintf( stderr, "ERROR: Unable to resolve assert arg values !\n");
		  exit( ERR_ASSERT_RESOLVE );			
		}

		if (isVerbose == TRUE) {
		  fprintf( stderr, ">>> Comparing:: Arg1: \"%s\"\n", arg1Value );
 		  fprintf( stderr, ">>>             Arg2: \"%s\"\n", arg2Value );
		}

		if ( areStringsEqual(arg1Value, arg2Value) == FALSE) {
		  fprintf( stderr, "ERROR: String values do not match !\n");
 		  fprintf( stderr, "   Arg1: \"%s\"\n", arg1Value );
 		  fprintf( stderr, "   Arg2: \"%s\"\n", arg2Value );
		  exit( ERR_ASSERT_MISMATCH );
		}
	}

	/////
	// Set UMASK
	if (isUmaskSet == TRUE) {
		if (isVerbose == TRUE) {
		  fprintf( stderr, ">>> Setting umask to %o\n", umaskValue );
		}
		umask( umaskValue );
	}

	/////
	// Set Working Directory
	if (isWorkingDirectorySet == TRUE) {
		if (isVerbose == TRUE) {
		  fprintf( stderr, ">>> Setting working directory to %s\n", workingDirectory );
		}

		// change dir to user home dir
		if ( chdir( workingDirectory ) != 0 ){
			fprintf( stderr, "ERROR: Failed to CHDIR to \"%d\"\n", workingDirectory );
			exit( ERR_CHDIR_FAILED );
		}
	}

	/////
	// Execute command
	if (isVerbose == TRUE) {
	  fprintf( stderr, ">>> Executing command \"%s\" (delimited by \"%s\")\n", command, delimiters );
	}

	// Parse command
	string_t tokenStart = command;
	string_t target_args[MAX_ARG_SIZE];

	for (i=0; i<MAX_ARG_SIZE; i++) {
		target_args[i] = parseNextStringToken(&tokenStart, delimiters);
		if (isVerbose == 1 && target_args[i] != NULL) {
			fprintf( stderr, ">>> Exec Arg%d = \"%s\"\n", i, target_args[i] );
		}
		if ((i+1) == MAX_ARG_SIZE && target_args[i] != NULL ) {
			fprintf( stderr, "ERROR: Too many arguments in \"-c\" - maximum allowed it %d\n", i, MAX_ARG_SIZE-1 );
			exit( ERR_COMMAND_TOO_MANY_ARGS );
		}
	}

	// exec command
        char **target_env   = environ;
	execve( target_args[0], target_args, target_env);
	exit( ERR_COMMAND_EXEC_FAILED );
}



// Return true if str1 is equal to str2
boolean_t areStringsEqual(const string_t str1, const string_t str2) {
	return ( strlen(str1) == strlen(str2) ) && ( memcmp(str1, str2, strlen(str2) ) == 0 );
}


// Returns true of str1 starts with str2
boolean_t startsWith(const string_t str1, const string_t str2) {
	return ( strlen(str1) >= strlen(str2) ) && ( memcmp(str1, str2, strlen(str2) ) == 0 );
}


// Returns a value of the string (doing lookup into environment variables)
string_t substring(const string_t str, int startPos, int length) {
	// Check length
	int strLength = strlen(str);
	if ((startPos+length) > strLength) {
		fprintf( stderr, "ERROR: Invalid args [startPos=%d, length=%d] for input string of length %d !\n", startPos, length, strLength );
		exit( ERR_INVALID_ARG );		
	}
	
	string_t outString;

	// Alloc memory
	if ( ( outString = (char*) malloc( (length + 1) * sizeof(char) ) ) == NULL ){
		fprintf( stderr, "ERROR: Not enough memory !\n" );
		exit( ERR_NOT_ENOUGH_MEMORY );
        }

        // fill the allocated memory with 0
       	memset( outString, 0, length + 1 );

        // copy substring into variable name striping "env{" prefix and "}" suffix
       	return strncpy( outString, str+startPos , length );
}


// Returns a value of the string (doing lookup into environment variables)
string_t getStringValue(const string_t str ) {
	string_t variableName;

	// Is length <6 return original string
	int length = strlen(str);
	if ( length < 6) {
		return str;
	}

	// Does not start with env{
	if ( startsWith(str, "env{") == FALSE) {
		return str;
	}

	// Does not end with }
	if ( str[length-1] != '}' ) {
		return NULL;
	}

	// Extract vaiableName
	variableName = substring(str, 4, length - 5);

        // Copy substring into variable name striping "env{" prefix and "}" suffix
       	variableName = strncpy( variableName, str+4, length-5 );

	// get environment variable
	string_t result = getenv(variableName);
       	#ifdef TRACE
          fprintf( stderr, "TRACE: Returning value %s of env-variable %s\n", result, variableName );
        #endif

	// Free memory
	free(variableName);

	// Return value
	return result;
}


// Parses the String-token form the sting pointed to be tokenStart 
string_t parseNextStringToken( string_t *tokenStart, const string_t delims ){
	string_t outString;

        #ifdef TRACE
       	  fprintf( stderr, "TRACE: Initial TokenString=\"%s\"\n", *tokenStart  );
        #endif

	// skip leading delimiters
	size_t length = strspn( *tokenStart, delims );
	*tokenStart = *tokenStart + length;

        #ifdef TRACE
       	  fprintf( stderr, "TRACE: Normalized TokenString=\"%s\"\n", *tokenStart  );
        #endif

	// If string starts with quote
	if ( startsWith(*tokenStart, "\"") == TRUE) {
		// skip Leading Quote
		*tokenStart = *tokenStart + 1;

		// calculate token length
	        length = strcspn( *tokenStart, "\"" );
	        #ifdef TRACE
			fprintf( stderr, "TRACE: Quote-Token length is %d\n", length );
	        #endif

		// copy substring
		outString = substring(*tokenStart, 0, length);

		// move tokenStart
		*tokenStart = *tokenStart + length;

		// skip Trailing Quote
		if (strncmp("\"", *tokenStart, 1) == 0) {
			*tokenStart = *tokenStart + 1;
		}
	}
	else {
		// calculate token length
	        length = strcspn( *tokenStart, delims );
	        #ifdef TRACE
			fprintf( stderr, "TRACE: Non-Quote token length is %d\n", length );
	        #endif

		// If length is 0 => No more tokens
		if (length == 0) {
		        #ifdef TRACE
		       	  fprintf( stderr, "TRACE: Parsing Completed - no more tokens - returning NULL\n" );
		        #endif
			return NULL;
		}
		
		// copy substring
		outString = substring(*tokenStart, 0, length);

		// move tokenStart
		*tokenStart = *tokenStart + length;
	}

        #ifdef TRACE
       	  fprintf( stderr, "TRACE: Parsing Completed token=\"%s\", new tokenStart=\"%s\"\n", outString, *tokenStart );
        #endif

        return outString;
}


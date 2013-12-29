/*
pam_trace - A simple PAM account module
Copyright (C) 2003 Andreas Gohr <a.gohr@web.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/



#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <security/pam_modules.h>

#include <syslog.h>
#include <string.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <malloc.h>
#include <stdarg.h>

#ifndef PAM_EXTERN
	#define PAM_EXTERN extern
#endif

// These are required to that the PAM infrastructure knows which functions are supported by this module
#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

#define MAX_STRING_SIZE 512

#ifdef PAM_MODULE_ENTRY
	PAM_MODULE_ENTRY("pam_trace");
#endif

static const void* get_pam_item(const pam_handle_t* pamh, int item_type) {
	const void *item;

	if (pam_get_item (pamh, item_type, &item) != PAM_SUCCESS) {
		return NULL;
	}
	return item;
	
}

static void format_pam_string(const char* name, const pam_handle_t* pamh, char* format, char* out) {
	const char *f, *q;
	const void *str = NULL;
	size_t len;
   	for (len = 0; *format != '\0' && len < MAX_STRING_SIZE-1; format++) {
		if (*format != '%' || format[1] == '\0') {
			out[len++] = *format;
   			continue;
        	}

		switch (*++format) {
			// Action
			case 'a':
				str = name;			
				break;
   			case 'H':
				str = get_pam_item(pamh, PAM_RHOST);
				break; 
			case 's':
				str = get_pam_item(pamh, PAM_SERVICE);
				break;
			case 't':
				str = get_pam_item(pamh, PAM_TTY);
  				break;
			case 'U':
				str = get_pam_item(pamh, PAM_RUSER);
  				break;
			case 'u':
				str = get_pam_item(pamh, PAM_USER);
  				break;
			default:
				out[len++] = *format;
				continue;
		}

		if (str == NULL) {
			continue;
		}

		// Append str to output
		for (q = str; *q != '\0' && len < MAX_STRING_SIZE - 1; ++q) {
			out[len++] = *q;
		}
	}
  	out[len] = '\0';
}

static void pam_trace(const char* name,
	pam_handle_t * pamh,
	int flags,
	int argc,
	const char *argv[] ) {

	// In case of PAM_SILENT flag - do nothing
	if (flags & PAM_SILENT) {
		return;
	}

	// Get the file and format
	char* format = NULL;
	int noprefix = 0;
	for (; argc-- > 0; ++argv)
  	{
		if (!strcmp(*argv, "noprefix")) {
			noprefix = 1;
		}
		else if (!strncmp (*argv, "format=", 7)) {
			format = (char*) (*argv + 7);
		}
	}

	if (format == NULL) {
		pam_syslog(pamh, LOG_ERR, "%s: format= was not provided !", name);
		return;
	}

	// Format output string
	char out[MAX_STRING_SIZE+2];
	format_pam_string(name, pamh, format, out);
	
	// Log status
	if (noprefix) {
		syslog(LOG_INFO, out);
	}
	else {
		pam_syslog(pamh, LOG_INFO, out);
	}
}


/* --------------------------- PAM functions --------------------------------*/

PAM_EXTERN int pam_sm_acct_mgmt ( pam_handle_t *pamh,
                                  int flags,
                                  int argc,
                                  const char *argv[] ) {
	pam_trace("acct_mgmt", pamh, flags, argc, argv);
	return PAM_IGNORE;
}

/* function for auth modules - ignore us! */
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh,
                                   int flags,
                                   int argc,
                                   const char **argv){
	pam_trace("authenticate", pamh, flags, argc, argv);
  	return PAM_IGNORE;
}

/* function for password modules - ignore us! */
PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh,
                                int flags,
                                int argc,
                                const char **argv){
	pam_trace("chauthtok", pamh, flags, argc, argv);
  	return PAM_IGNORE;
}

/* functions for session modules - ignor us */
PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh,
                                   int flags,
                                   int argc,
                                   const char **argv){
	pam_trace("open_session", pamh, flags, argc, argv);
	return PAM_IGNORE;
}
PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh,
                                    int flags,
                                    int argc,
                                    const char **argv){
	pam_trace("close_session", pamh, flags, argc, argv);
  	return PAM_IGNORE;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh,
                                 int flags,
                                 int argc,
                                 const char **argv){
	pam_trace("setcred", pamh, flags, argc, argv);
	return PAM_IGNORE;
}


#ifdef PAM_STATIC
	/* static module data */
	struct pam_module _pam_trace_modstruct = {
  		"pam_trace",
  		pam_sm_authenticae,
		pam_sm_setcred,
		pam_sm_acct_mgmt,
		pam_sm_open_session,
		pam_sm_close_session,
		pam_sm_chauthtok,
	}; 
#endif

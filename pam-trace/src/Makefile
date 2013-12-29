# VARIABLES
CC		= gcc
LD		= ld
CFLAGS		= -g -O2 -fPIC
DEFS		= 
#DEFS		= -DTRACE

#MACRO DEFINITIONS




# Linking information
pam_trace.so : pam_trace.o
	${LD} -x --shared -o pam_trace.so pam_trace.o 

pam_trace.o : pam_trace.c
	${CC} ${CFLAGS} ${DEFS} -c pam_trace.c




# make all
all:
	pam_trace.so

# clean object files
clean:
	rm pam_trace.so pam_trace.o

# Install the module
install:
	install -o root -g root -m 755 pam_trace.so /lib/security/pam_trace.so

//
// logging stuff
//

#ifndef LOGGER_H
#define LOGGER_H

#include <string.h>
#include <errno.h>

enum loglevel
{
	CARACAD_LOG_FATAL,
	CARACAD_LOG_DEBUG,
	CARACAD_LOG_INFO,
	CARACAD_LOG_LOGICAL,
	CARACAD_LOG_INTEGRITY,
	CARACAD_LOG_ADDRESSING,
	CARACAD_LOG_CLIENT,
	CARACAD_LOG_MAX
};

extern void log(int level, char* format,...);
extern void log_init(char *filename,char* progname);
extern void log_set_level(int level,int enabled);

#define error_exit(message) \
	{ log(CARACAD_LOG_FATAL,"%s.  %s(),%s,line %d",strerror(errno),message,__FILE__,__LINE__); \
	exit(1); }

//void  debug(int level,const char *fmt,...) __attribute__((format(printf, 2, 3)));
void  debug(int level,const char *fmt,...);

#endif

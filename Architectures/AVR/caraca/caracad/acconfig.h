/*      $Id: acconfig.h,v 1.2 2000/08/21 16:14:40 skr Exp $      */

/*
 *  are you editing the correct file?
 *  
 *  acconfig.h  - changes for distribution
 *                you must run autoheader / configure
 *  config.h.in - changes specific to your installation
 *                these will be lost if you run autoheader
 *  config.h    - changes to this configuration
 *                these will be lost if you run configure
 */

/* config file names - beneath $HOME */
#define CFG_USER	".caracarc"

/* log files */
#define LOGDIR		"/var/log"
#define LOG_LIRCD	"caracad"

/* pid file */
#define PIDDIR          "/var/run"
#define PID_CARACAD     "caracad.pid"

/*
 * SHARED DEFINITIONS - things you should change
 */

#define CAN_SYSLOG
#define DEBUG
#define OPENLOG_OPTIONS LOG_PID

/*********************************************************************
 *                                                                   *
 * You probably do not need to edit anything below this point        *
 *                                                                   *
 *********************************************************************/

#define MAX_ARGC      100
#define MAX_ZEICHEN   100
#define REALLOC_CNT   10
#define DEFAULT_CAN_DEV "/dev/can"

/*
 * below here are defines managed by autoheader / autoconf
 */

@TOP@

/* define in maintainer mode */
#undef MAINTAINER_MODE

/* Define to use long long IR codes */
#undef LONG_IR_CODE

/* Define to enable debugging output */
#undef DEBUG

/* Define to run daemons as daemons */
#undef DAEMONIZE

/* Define if the libirman library is installed */
#undef HAVE_LIBIRMAN

/* Define if the software only test version of libirman is installed */
#undef HAVE_LIBIRMAN_SW

/* Define if the complete vga libraries (vga, vgagl) are installed */
#undef HAVE_LIBVGA

/* define if you have vsyslog( prio, fmt, va_arg ) */
#undef HAVE_VSYSLOG

/* define if you want to log to syslog instead of logfile */
#undef USE_SYSLOG

/* syslog facility to use */
#define LIRC_SYSLOG		LOG_DAEMON

/* system configuration directory */
#define SYSCONFDIR		"/etc"

/* device files directory */
#define DEVDIR			"/dev"

/* This should only be set by configure */
#define PACKAGE			"unset"

/* This should only be set by configure */
#define VERSION			"0.0.0"

@BOTTOM@

/*
 * compatibility and useability defines
 */

#define PIDFILE                 PIDDIR "/" PID_CARACAD

/* end of acconfig.h */

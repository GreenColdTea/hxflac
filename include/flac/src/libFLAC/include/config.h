#ifndef CONFIG_H
#define CONFIG_H

/* ver. info */
#define PACKAGE_VERSION "1.5.0"

/* standard headers */
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_INTTYPES_H 1

/* funcs */
#define HAVE_STRDUP 1
#define HAVE_VSNPRINTF 1

/* disable assembly */
#define FLAC__NO_ASM 1

/* Platform-specific configurations */
#ifdef _WIN32
/* Windows configuration */
#define _CRT_SECURE_NO_WARNINGS 1

/* Windows doesn't have these */
#define HAVE_UNISTD_H 0
#define HAVE_FCNTL_H 0
#define HAVE_SYS_TYPES_H 0
#define HAVE_SYS_STAT_H 0
#define HAVE_FSEEKO 0
#define HAVE_FTELLO 0
#define HAVE_LRINT 0
#define HAVE_LRINTF 0
#define HAVE_LROUND 0

#else
/* Linux/macOS configuration */
#define HAVE_UNISTD_H 1
#define HAVE_FCNTL_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_FSEEKO 1
#define HAVE_FTELLO 1
#define HAVE_LRINT 1
#define HAVE_LRINTF 1
#define HAVE_LROUND 1

#endif

#endif /* CONFIG_H */
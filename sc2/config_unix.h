/*
 * 
 * This file contains some compile-time configuration options for 
 * cross-compiling from a unix system to the Wii
 *
 */

#ifndef CONFIG_UNIX_H_
#define CONFIG_UNIX_H_

/* Directory where the UQM game data is located */
//#define CONTENTDIR "sd:/apps/uqm/content"
#define CONTENTDIR "content"

/* Directory where game data will be stored */
#define USERDIR "sd:/apps/uqm/user"

/* Directory where config files will be stored */
#define CONFIGDIR USERDIR

/* Directory where supermelee teams will be stored */
#define MELEEDIR "sd:/apps/uqm/teams/"

/* Directory where save games will be stored */
#define SAVEDIR "sd:/apps/uqm/save/"

/* Defined if words are stored with the most significant byte first */
#define WORDS_BIGENDIAN

/* Defined if your system has readdir_r of its own */
#define HAVE_READDIR_R

/* Defined if your system has setenv of its own */
#define HAVE_SETENV

/* Defined if your system has strupr of its own */
#undef HAVE_STRUPR

/* Defined if your system has strcasecmp of its own */
#define HAVE_STRCASECMP_UQM
		// Not using "HAVE_STRCASECMP" as that conflicts with SDL.

/* Defined if your system has stricmp of its own */
#undef HAVE_STRICMP

/* Defined if your system has getopt_long */
#define HAVE_GETOPT_LONG

/* Defined if your system has iswgraph of its own*/
#define HAVE_ISWGRAPH

/* Defined if your system has wchar_t of its own */
#define HAVE_WCHAR_T

/* Defined if your system has wint_t of its own */
#define HAVE_WINT_T

/* Defined if your system has _Bool of its own */
#define HAVE__BOOL

/* using tremor for oggvorbis */
#define OVCODEC_TREMOR

/* using HAVE_DRIVE_LETTERS to deal with SD Card paths */
#define HAVE_DRIVE_LETTERS
// TODO: change to more appropriate name like SD_CARD_PATHS

// named synchronization, seems to be required by the threading system
#define NAMED_SYNCHRO

#endif  /* CONFIG_UNIX_H_ */


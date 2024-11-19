#ifndef _APP_VERSION_H_
#define _APP_VERSION_H_

/* The template values come from cmake/version.cmake
 * BUILD_VERSION related template values will be 'git describe',
 * alternatively user defined BUILD_VERSION.
 */

/* #undef ZEPHYR_VERSION_CODE */
/* #undef ZEPHYR_VERSION */

#define APPVERSION                   0x1050304
#define APP_VERSION_NUMBER           0x10503
#define APP_VERSION_MAJOR            1
#define APP_VERSION_MINOR            5
#define APP_PATCHLEVEL               3
#define APP_TWEAK                    4
#define APP_VERSION_STRING           "1.5.3-unstable"
#define APP_VERSION_EXTENDED_STRING  "1.5.3-unstable+4"
#define APP_VERSION_TWEAK_STRING     "1.5.3+4"

#define APP_BUILD_VERSION 04db28b812f0


#endif /* _APP_VERSION_H_ */

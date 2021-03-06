/*
 * This file is part of NGWorld.
 * (C) Copyright 2016 DLaboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Detection of operating system is modified from the source code of
 * Qt 3.1.2, which is published under the terms of LGPL version 2.1.
 */

#ifndef _FUNDAMENTAL_MACROS_H_
#define _FUNDAMENTAL_MACROS_H_

/*
   The operating system, must be one of: (NGWORLD_OS_x)

     MACX	- Mac OS X
     MAC9	- Mac OS 9
     MSDOS	- MS-DOS and Windows
     OS2	- OS/2
     OS2EMX	- XFree86 on OS/2 (not PM)
     WIN32	- Win32 (Windows 95/98/ME and Windows NT/2000/XP)
     CYGWIN	- Cygwin
     SOLARIS	- Sun Solaris
     HPUX	- HP-UX
     ULTRIX	- DEC Ultrix
     LINUX	- Linux
     FREEBSD	- FreeBSD
     NETBSD	- NetBSD
     OPENBSD	- OpenBSD
     BSDI	- BSD/OS
     IRIX	- SGI Irix
     OSF	- HP Tru64 UNIX
     SCO	- SCO OpenServer 5
     UNIXWARE	- UnixWare 7, Open UNIX 8
     AIX	- AIX
     HURD	- GNU Hurd
     DGUX	- DG/UX
     RELIANT	- Reliant UNIX
     DYNIX	- DYNIX/ptx
     QNX	- QNX
     QNX6	- QNX RTP 6.1
     LYNX	- LynxOS
     BSD4	- Any BSD 4.4 system
     UNIX	- Any UNIX BSD/SYSV system
*/

#if defined(__APPLE__) && defined(__GNUC__)
#  define NGWORLD_OS_MACX
#elif defined(__MACOSX__)
#  define NGWORLD_OS_MACX
#elif defined(macintosh)
#  define NGWORLD_OS_MAC9
#elif defined(__CYGWIN__)
#  define NGWORLD_OS_CYGWIN
#elif defined(MSDOS) || defined(_MSDOS)
#  define NGWORLD_OS_MSDOS
#elif defined(__OS2__)
#  if defined(__EMX__)
#    define NGWORLD_OS_OS2EMX
#  else
#    define NGWORLD_OS_OS2
#  endif
#elif !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define NGWORLD_OS_WIN32
#  define NGWORLD_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  define NGWORLD_OS_WIN32
#elif defined(__MWERKS__) && defined(__INTEL__)
#  define NGWORLD_OS_WIN32
#elif defined(__sun) || defined(sun)
#  define NGWORLD_OS_SOLARIS
#elif defined(hpux) || defined(__hpux)
#  define NGWORLD_OS_HPUX
#elif defined(__ultrix) || defined(ultrix)
#  define NGWORLD_OS_ULTRIX
#elif defined(sinix)
#  define NGWORLD_OS_RELIANT
#elif defined(__linux__) || defined(__linux)
#  define NGWORLD_OS_LINUX
#elif defined(__FreeBSD__)
#  define NGWORLD_OS_FREEBSD
#  define NGWORLD_OS_BSD4
#elif defined(__NetBSD__)
#  define NGWORLD_OS_NETBSD
#  define NGWORLD_OS_BSD4
#elif defined(__OpenBSD__)
#  define NGWORLD_OS_OPENBSD
#  define NGWORLD_OS_BSD4
#elif defined(__bsdi__)
#  define NGWORLD_OS_BSDI
#  define NGWORLD_OS_BSD4
#elif defined(__sgi)
#  define NGWORLD_OS_IRIX
#elif defined(__osf__)
#  define NGWORLD_OS_OSF
#elif defined(_AIX)
#  define NGWORLD_OS_AIX
#elif defined(__Lynx__)
#  define NGWORLD_OS_LYNX
#elif defined(__GNU_HURD__)
#  define NGWORLD_OS_HURD
#elif defined(__DGUX__)
#  define NGWORLD_OS_DGUX
#elif defined(__QNXNTO__)
#  define NGWORLD_OS_QNX6
#elif defined(__QNX__)
#  define NGWORLD_OS_QNX
#elif defined(_SEQUENT_)
#  define NGWORLD_OS_DYNIX
#elif defined(_SCO_DS)                   /* SCO OpenServer 5 + GCC */
#  define NGWORLD_OS_SCO
#elif defined(__USLC__)                  /* all SCO platforms + UDK or OUDK */
#  define NGWORLD_OS_UNIXWARE
#  define NGWORLD_OS_UNIXWARE7
#elif defined(__svr4__) && defined(i386) /* Open UNIX 8 + GCC */
#  define NGWORLD_OS_UNIXWARE
#  define NGWORLD_OS_UNIXWARE7
#else
#  error "NGWorld failed to detect the operating system."
#endif

#if defined(NGWORLD_OS_MAC9) || defined(NGWORLD_OS_MACX)
#  define NGWORLD_OS_MAC
#endif

#if defined(NGWORLD_OS_MAC9) || defined(NGWORLD_OS_MSDOS) || defined(NGWORLD_OS_OS2) || defined(NGWORLD_OS_WIN32) || defined(NGWORLD_OS_WIN64)
#  undef NGWORLD_OS_UNIX
#elif !defined(NGWORLD_OS_UNIX)
#  define NGWORLD_OS_UNIX
#endif

#if defined(NGWORLD_OS_MSDOS) || defined(NGWORLD_OS_WIN32) || defined(NGWORLD_OS_WIN64)
#  define NGWORLD_OS_WINDOWS
#endif

#endif

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

############### INITIALIZE EVERYTHING ######################

AC_INIT([PitchTempoPlayer],
	[1.2.0],
	[nadaeck@hotmail.com], 
	[pitchtempoplayer],
	[http://www.sourceforge.net/ptplayer])
AC_CONFIG_SRCDIR([Makefile.am])

AM_INIT_AUTOMAKE([-Wall])

AC_PROG_CC
AM_PROG_CC_C_O

AC_CONFIG_HEADERS([src/config.h])


################# CHECK FOR LIBRAIRIES ######################

# Check and link for GTK3
PKG_CHECK_MODULES([GTK3], [gtk+-3.0], [
	gtk=3
	CFLAGS="$CFLAGS $GTK3_CFLAGS"
	LIBS="$LIBS $GTK3_LIBS"
	AC_DEFINE([GTK3], [1], [Define to 1 if you have the `GTK3' library])
	], [gtk=0])

# Check and link for GSTREAMER-1.0
PKG_CHECK_MODULES([GSTREAMER], [gstreamer-1.0], [
				CFLAGS="$CFLAGS $GSTREAMER_CFLAGS"
   				LIBS="$LIBS $GSTREAMER_LIBS"
				], [AC_MSG_ERROR([GSTREAMER (gstreamer-1.0) library (libgstreamer) is required])])

# Check and link for JACK 
AC_CHECK_LIB([jack], [jack_client_open], [],
	[AC_MSG_ERROR([Jack library (libjack) is required for this program])])

# Check and link for LIBM 
AC_CHECK_LIB([m], [cos], [],
	[AC_MSG_ERROR([Libm library is required for this program])])

# Checks for header files.		
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for library functions.
AC_CHECK_FUNCS([strchr strstr strtol])

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/cplayer.desktop])

AC_OUTPUT








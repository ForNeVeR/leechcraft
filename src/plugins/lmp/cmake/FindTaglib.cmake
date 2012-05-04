# - Try to find the Taglib library
# Once done this will define
#
#  TAGLIB_FOUND - system has the taglib library
#  TAGLIB_CFLAGS - the taglib cflags
#  TAGLIB_LIBRARIES - The libraries needed to use taglib

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT TAGLIB_MIN_VERSION)
  set(TAGLIB_MIN_VERSION "1.4")
endif(NOT TAGLIB_MIN_VERSION)

if(NOT WIN32)
    find_program(TAGLIBCONFIG_EXECUTABLE NAMES taglib-config PATHS
       ${BIN_INSTALL_DIR}
    )
endif(NOT WIN32)

#reset vars
set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)

# if taglib-config has been found
if(TAGLIBCONFIG_EXECUTABLE)

  exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)

  if(TAGLIB_VERSION STRLESS "${TAGLIB_MIN_VERSION}")
     message(STATUS "TagLib version not found: version searched :${TAGLIB_MIN_VERSION}, found ${TAGLIB_VERSION}")
     set(TAGLIB_FOUND FALSE)
  else(TAGLIB_VERSION STRLESS "${TAGLIB_MIN_VERSION}")

     exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LIBRARIES)

     exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)

     if(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
        set(TAGLIB_FOUND TRUE)
        message(STATUS "Found taglib: ${TAGLIB_LIBRARIES}")
     endif(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
     string(REGEX REPLACE " *-I" ";" TAGLIB_INCLUDES "${TAGLIB_CFLAGS}")
  endif(TAGLIB_VERSION STRLESS "${TAGLIB_MIN_VERSION}") 
  mark_as_advanced(TAGLIB_CFLAGS TAGLIB_LIBRARIES TAGLIB_INCLUDES)

else(TAGLIBCONFIG_EXECUTABLE)


  include(FindPackageHandleStandardArgs)

  IF (WIN32)
	IF (MSVC)
		#MSVS 2010
		IF (MSVC_VERSION LESS 1600)
			MESSAGE(FATAL_ERROR "We currently support only MSVC 2010 version")
		ENDIF (MSVC_VERSION LESS 1600)
	ENDIF (MSVC)
	IF (NOT DEFINED TAGLIB_DIR)
		IF (TAGLIB_FIND_REQUIRED)
			MESSAGE(FATAL_ERROR "Please set TAGLIB_DIR variable")
		ELSE (TAGLIB_FIND_REQUIRED)
			MESSAGE(STATUS "Please set TAGLIB_DIR variable for taglib support")
		ENDIF (TAGLIB_FIND_REQUIRED)
	ENDIF (NOT DEFINED TAGLIB_DIR)
	SET (TAGLIB_INCLUDE_WIN32 ${TAGLIB_DIR})

    SET (PROBE_DIR_Debug
		${TAGLIB_DIR}/build/taglib/Debug)
	SET (PROBE_DIR_Release
		${TAGLIB_DIR}/build/taglib/MinSizeRel ${TAGLIB_DIR}/build/taglib/Release)

	FIND_LIBRARY (TAGLIB_LIBRARY_DEBUG NAMES tag.lib PATHS ${PROBE_DIR_Debug})
	FIND_LIBRARY (TAGLIB_LIBRARY_RELEASE NAMES tag.lib PATHS ${PROBE_DIR_Release})
	win32_tune_libs_names (TAGLIB)
	
	find_path(TAGLIB_INCLUDES
      NAMES
      taglib/tag.h
      PATH_SUFFIXES taglib
      PATHS
      ${KDE4_INCLUDE_DIR}
      ${INCLUDE_INSTALL_DIR}
	  ${TAGLIB_INCLUDE_WIN32}
    )
  ELSE (WIN32)
    include(FindLibraryWithDebug)
  
    find_library_with_debug(TAGLIB_LIBRARIES
      WIN32_DEBUG_POSTFIX d
      NAMES tag
      PATHS
      ${KDE4_LIB_DIR}
      ${LIB_INSTALL_DIR}
    )
	
	find_path(TAGLIB_INCLUDES
      NAMES
      tag.h
      PATH_SUFFIXES taglib
      PATHS
      ${KDE4_INCLUDE_DIR}
      ${INCLUDE_INSTALL_DIR}
    )
  ENDIF (WIN32)
  
  find_package_handle_standard_args(Taglib DEFAULT_MSG 
                                    TAGLIB_INCLUDES TAGLIB_LIBRARIES)
endif(TAGLIBCONFIG_EXECUTABLE)


if(TAGLIB_FOUND)
  if(NOT Taglib_FIND_QUIETLY AND TAGLIBCONFIG_EXECUTABLE)
    message(STATUS "Taglib found: ${TAGLIB_LIBRARIES}")
  endif(NOT Taglib_FIND_QUIETLY AND TAGLIBCONFIG_EXECUTABLE)
else(TAGLIB_FOUND)
  if(Taglib_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Taglib")
  endif(Taglib_FIND_REQUIRED)
endif(TAGLIB_FOUND)


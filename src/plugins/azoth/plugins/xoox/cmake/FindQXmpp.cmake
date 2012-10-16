# Find QXmpp library

# Note: On Win32 QXmpp_DIR needs point to Qxmpp install dir 
# QXmpp_INCLUDE_DIR
# QXmpp_LIBRARIES
# QXmpp_FOUND
# QXmpp_DIR

# Copyright (c) 2012 Dimtriy Ryazantcev <DJm00n@mail.ru>

if (QXmpp_INCLUDE_DIR AND QXmpp_LIBRARIES)
  # already in cache
  set(QXmpp_FOUND TRUE)
else (QXmpp_INCLUDE_DIR AND QXmpp_LIBRARIES)
  if (NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_QXmpp QUIET qxmpp)
    set(QXmpp_DEFINITIONS ${PC_QXmpp_CFLAGS_OTHER})
	find_library(QXmpp_LIBRARIES NAMES qxmpp HINTS ${PC_QXmpp_LIBDIR} ${PC_QXmpp_LIBRARY_DIRS})
  else (NOT WIN32)
    if (NOT DEFINED QXmpp_DIR)
			if (QXmpp_FIND_REQUIRED)
				message (FATAL_ERROR "Please set QXmpp_DIR variable")
			else (QXmpp_FIND_REQUIRED)
				message (STATUS "Please set QXmpp_DIR variable for QXmpp support")
			endif (QXmpp_FIND_REQUIRED)
	endif (NOT DEFINED QXmpp_DIR)
    # on Windows we are looking for installed QXmpp in QXmpp_DIR variable
    set(QXmpp_LIB_WIN32 ${QXmpp_DIR}/lib)
	set(QXmpp_INCLUDE_WIN32 ${QXmpp_DIR}/include)
  
	find_library(QXmpp_LIBRARIES_RELEASE NAMES qxmpp0 qxmpp HINTS ${PC_QXmpp_LIBDIR} ${PC_QXmpp_LIBRARY_DIRS} ${QXmpp_LIB_WIN32})
    find_library(QXmpp_LIBRARIES_DEBUG NAMES qxmpp_d0 qxmpp_d HINTS ${PC_QXmpp_LIBDIR} ${PC_QXmpp_LIBRARY_DIRS} ${QXmpp_LIB_WIN32})
	
	IF(QXmpp_LIBRARIES_RELEASE AND QXmpp_LIBRARIES_DEBUG)
        # both libs found
        SET(QXmpp_LIBRARIES optimized ${QXmpp_LIBRARIES_RELEASE}
                            debug     ${QXmpp_LIBRARIES_DEBUG})
      ELSE(QXmpp_LIBRARIES_RELEASE AND QXmpp_LIBRARIES_DEBUG)
        IF(QXmpp_LIBRARIES_RELEASE)
          # only release found
          SET(QXmpp_LIBRARIES ${QXmpp_LIBRARIES_RELEASE})
        ELSE(QXmpp_LIBRARIES_RELEASE)
          # only debug (or nothing) found
          SET(QXmpp_LIBRARIES ${QXmpp_LIBRARIES_DEBUG})
        ENDIF(QXmpp_LIBRARIES_RELEASE)
      ENDIF(QXmpp_LIBRARIES_RELEASE AND QXmpp_LIBRARIES_DEBUG)
	  MARK_AS_ADVANCED(QXmpp_LIBRARIES_RELEASE)
      MARK_AS_ADVANCED(QXmpp_LIBRARIES_DEBUG)
  endif (NOT WIN32)
  			  
  find_path(QXmpp_INCLUDE_DIR
			NAMES
			QXmppClient.h
            HINTS ${PC_QXmpp_INCLUDEDIR} ${PC_QXmpp_INCLUDE_DIRS} ${QXmpp_INCLUDE_WIN32}
            PATH_SUFFIXES qxmpp qxmpp-dev)
			
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(QXmpp  DEFAULT_MSG  QXmpp_LIBRARIES QXmpp_INCLUDE_DIR)
  
  mark_as_advanced(QXmpp_INCLUDE_DIR QXmpp_LIBRARIES)
  
endif (QXmpp_INCLUDE_DIR AND QXmpp_LIBRARIES)

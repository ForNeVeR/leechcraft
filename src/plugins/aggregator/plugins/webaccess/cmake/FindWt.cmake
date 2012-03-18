# Find Wt includes and libraries

#
# Wt_INCLUDE_DIR
# Wt_LIBRARIES  - Release libraries
# Wt_DEBUG_LIBRARIES  - Debug libraries
# Wt_DEBUG_FOUND  - True if debug libraries found
# Wt_LIBRARY_FOUND - True if core Wt library found
# Wt_EXT_LIBRARY_FOUND - True if ExtJS Wt library found
# Wt_HTTP_LIBRARY_FOUND - True if HTTP Wt library found
# Wt_FCGI_LIBRARY_FOUND - True if FCGI Wt library found


#
# Copyright (c) 2007, Pau Garcia i Quiles, <pgquiles@elpauer.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Some modifications, copyright (c) 2008, Rudoy Georg,
# 0xd34df00d@gmail.com>

FIND_PATH( Wt_INCLUDE_DIR NAMES WObject PATHS ENV PATH PATH_SUFFIXES include wt Wt )

SET( Wt_FIND_COMPONENTS Release Debug )

IF( Wt_INCLUDE_DIR )
        FIND_LIBRARY( Wt_LIBRARY NAMES wt PATHS PATH PATH_SUFFIXES lib lib-release lib_release )
        FIND_LIBRARY( Wt_EXT_LIBRARY NAMES wtext PATHS PATH PATH_SUFFIXES lib lib-release lib_release )
        FIND_LIBRARY( Wt_HTTP_LIBRARY NAMES wthttp PATHS PATH PATH_SUFFIXES lib lib-release lib_release )
        FIND_LIBRARY( Wt_FCGI_LIBRARY NAMES wtfcgi PATHS PATH PATH_SUFFIXES lib lib-release lib_release )

        FIND_LIBRARY( Wt_DEBUG_LIBRARY NAMES wtd PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )
        FIND_LIBRARY( Wt_EXT_DEBUG_LIBRARY NAMES wtextd PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )
        FIND_LIBRARY( Wt_HTTP_DEBUG_LIBRARY NAMES wthttpd PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )
        FIND_LIBRARY( Wt_FCGI_DEBUG_LIBRARY NAMES wtfcgid PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )

        IF( Wt_LIBRARY OR Wt_EXT_LIBRARY OR Wt_HTTP_LIBRARY OR Wt_FCGI_LIBRARY )
				SET( Wt_FOUND TRUE )
				SET( Wt_FIND_REQUIRED_Release TRUE )
				SET( Wt_LIBRARIES ${Wt_LIBRARY} ${Wt_EXT_LIBRARY} ${Wt_HTTP_LIBRARY} ${Wt_FCGI_LIBRARY} )
        ENDIF( Wt_LIBRARY OR Wt_EXT_LIBRARY OR Wt_HTTP_LIBRARY OR Wt_FCGI_LIBRARY )

		iF (Wt_LIBRARY)
			SET (Wt_LIBRARY_FOUND TRUE)
		ENDIF (Wt_LIBRARY)
		iF (Wt_EXT_LIBRARY)
			SET (Wt_EXT_LIBRARY_FOUND TRUE)
		ENDIF (Wt_EXT_LIBRARY)
		iF (Wt_HTTP_LIBRARY)
			SET (Wt_HTTP_LIBRARY_FOUND TRUE)
		ENDIF (Wt_HTTP_LIBRARY)
		iF (Wt_FCGI_LIBRARY)
			SET (Wt_FCGI_LIBRARY_FOUND TRUE)
		ENDIF (Wt_FCGI_LIBRARY)

        IF( Wt_DEBUG_LIBRARY AND Wt_EXT_DEBUG_LIBRARY AND Wt_HTTP_DEBUG_LIBRARY AND Wt_FCGI_DEBUG_LIBRARY )
                SET( Wt_DEBUG_FOUND TRUE )
		SET( Wt_FIND_REQUIRED_Debug TRUE )
                SET( Wt_DEBUG_LIBRARIES ${Wt_DEBUG_LIBRARY} ${Wt_EXT_DEBUG_LIBRARY} ${Wt_HTTP_DEBUG_LIBRARY} ${Wt_FCGI_DEBUG_LIBRARY} )
        ENDIF( Wt_DEBUG_LIBRARY AND Wt_EXT_DEBUG_LIBRARY AND Wt_HTTP_DEBUG_LIBRARY AND Wt_FCGI_DEBUG_LIBRARY )

        IF(Wt_FOUND)
                IF (NOT Wt_FIND_QUIETLY)
                        MESSAGE(STATUS "Found the Wt libraries at ${Wt_LIBRARIES}")
                        MESSAGE(STATUS "Found the Wt headers at ${Wt_INCLUDE_DIR}")
                ENDIF (NOT Wt_FIND_QUIETLY)
        ELSE(Wt_FOUND)
                IF(Wt_FIND_REQUIRED)
                        MESSAGE(FATAL_ERROR "Could NOT find Wt")
                ENDIF(Wt_FIND_REQUIRED)
        ENDIF(Wt_FOUND)

        IF(Wt_DEBUG_FOUND)
                IF (NOT Wt_FIND_QUIETLY)
                        MESSAGE(STATUS "Found the Wt debug libraries at ${Wt_DEBUG_LIBRARIES}")
                        MESSAGE(STATUS "Found the Wt debug headers at ${Wt_INCLUDE_DIR}")
                ENDIF (NOT Wt_FIND_QUIETLY)
        ELSE(Wt_DEBUG_FOUND)
                IF(Wt_FIND_REQUIRED_Debug)
                        MESSAGE(FATAL_ERROR "Could NOT find Wt debug libraries")
                ENDIF(Wt_FIND_REQUIRED_Debug)
        ENDIF(Wt_DEBUG_FOUND)

ENDIF( Wt_INCLUDE_DIR )

cmake_minimum_required (VERSION 2.8)
project (leechcraft_netstoremanager_googledrive)
include (InitLCPlugin OPTIONAL)

if (NOT USE_QT5)
	find_package (QJSON REQUIRED)
else ()
	unset (QJSON_INCLUDE_DIR)
	unset (QJSON_LIBRARIES)
endif ()

include_directories (${NETSTOREMANAGER_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)

set (GOOGLEDRIVE_SRCS
	account.cpp
	authmanager.cpp
	core.cpp
	drivemanager.cpp
	googledrive.cpp
	uploadmanager.cpp
	xmlsettingsmanager.cpp
	)

set (GOOGLEDRIVE_RESOURCES netstoremanagergoogledriveresources.qrc)

CreateTrs ("netstoremanager_googledrive" "en;ru_RU" GOOGLEDRIVE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("netstoremanager_googledrive" "en;ru_RU" "${GOOGLEDRIVE_SRCS}" "" "nsmgoogledrivesettings.xml")

QtAddResources (GOOGLEDRIVE_RCCS ${GOOGLEDRIVE_RESOURCES})

add_library (leechcraft_netstoremanager_googledrive SHARED
	${GOOGLEDRIVE_COMPILED_TRANSLATIONS}
	${GOOGLEDRIVE_SRCS}
	${GOOGLEDRIVE_UIS_H}
	${GOOGLEDRIVE_RCCS}
	)
target_link_libraries (leechcraft_netstoremanager_googledrive
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)

install (TARGETS leechcraft_netstoremanager_googledrive DESTINATION ${LC_PLUGINS_DEST})
install (FILES nsmgoogledrivesettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_netstoremanager_googledrive Network Widgets)

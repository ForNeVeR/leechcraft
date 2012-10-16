#
# Versions settings overrides for Windows dll/exe file version resource.
# These values are compiled into the dll and exe files.
#
# The settings override precedence from lowest to highest:
# 1. CPACK settings from cpp/CMakeLists.txt
# 2. Global settings from this file
# 3. Command line version number (only) from add_msvc_version_full call
# 4. Per-project settings from this file
#

#
# Specification of global settings for all projects.
#

set ("winver_PACKAGE_NAME"         "leechcraft")
set ("winver_DESCRIPTION_SUMMARY"  "LeechCraft")
#set ("winver_FILE_VERSION_N1"      "0")
#set ("winver_FILE_VERSION_N2"      "4")
#set ("winver_FILE_VERSION_N3"      "5")
#set ("winver_FILE_VERSION_N4"      "0")
#set ("winver_PRODUCT_VERSION_N1"   "0")
#set ("winver_PRODUCT_VERSION_N2"   "4")
#set ("winver_PRODUCT_VERSION_N3"   "5")
#set ("winver_PRODUCT_VERSION_N4"   "0")
set ("winver_LEGAL_COPYRIGHT"      "Copyright (C) 2006-2012 Georg Rudoy <0xd34df00d@gmail.com>")
set ("winver_COMPANY_NAME"         "LeechCraft Developers")


#
# Specification of per-project settings:
#
# set ("winver_${projectName}_FileVersionBinary"    "0,4,5,0")
# set ("winver_${projectName}_ProductVersionBinary" "0,4,5,0")
# set ("winver_${projectName}_FileVersionString"    "0, 4, 5, 0")
# set ("winver_${projectName}_ProductVersionString" "0, 4, 5, 0")
# set ("winver_${projectName}_FileDescription"      "leechcraft-lcutil Library")
# set ("winver_${projectName}_LegalCopyright"       "")
# set ("winver_${projectName}_InternalName"         "lcutil")
# set ("winver_${projectName}_OriginalFilename"     "lcutil.dll")
# set ("winver_${projectName}_ProductName"          "LeechCraft")

set ("winver_leechcraft_FileDescription"            "LeechCraft is a free open source cross-platform modular internet-client.")
set ("winver_lcutil_FileDescription"                "Basic LeechCraft functionality.")
set ("winver_xmlsettingsdialog_FileDescription"     "XmlSettingsDialog provides a simple but powerful and abstract way to create settings dialogs from XML files and store their (and not only).")



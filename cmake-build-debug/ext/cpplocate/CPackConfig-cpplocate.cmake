# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_ARCHIVE_COMPONENT_INSTALL "OFF")
set(CPACK_BUILD_SOURCE_DIRS "/Users/sze/Desktop/CS5625/Starter22;/Users/sze/Desktop/CS5625/Starter22/cmake-build-debug")
set(CPACK_CMAKE_GENERATOR "Ninja")
set(CPACK_COMMAND "/Applications/CLion.app/Contents/bin/cmake/mac/bin/cpack")
set(CPACK_COMPONENTS_ALL "meta;runtime_cpp;dev_cpp;runtime_c;dev_c")
set(CPACK_COMPONENTS_ALL_SET_BY_USER "TRUE")
set(CPACK_COMPONENT_DEV_CPP_DEPENDS "runtime_cpp")
set(CPACK_COMPONENT_DEV_CPP_DESCRIPTION "Development files for cpplocate C++ library")
set(CPACK_COMPONENT_DEV_CPP_DISPLAY_NAME "C++ development files")
set(CPACK_COMPONENT_DEV_C_DEPENDS "runtime_c")
set(CPACK_COMPONENT_DEV_C_DESCRIPTION "Development files for cpplocate C library")
set(CPACK_COMPONENT_DEV_C_DISPLAY_NAME "C development files")
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY "OFF")
set(CPACK_COMPONENT_META_DESCRIPTION "Meta information for cpplocate library")
set(CPACK_COMPONENT_META_DISPLAY_NAME "cpplocate library")
set(CPACK_COMPONENT_RUNTIME_CPP_DESCRIPTION "Runtime components for cpplocate C++ library")
set(CPACK_COMPONENT_RUNTIME_CPP_DISPLAY_NAME "cpplocate C++ library")
set(CPACK_COMPONENT_RUNTIME_C_DESCRIPTION "Runtime components for cpplocate C library")
set(CPACK_COMPONENT_RUNTIME_C_DISPLAY_NAME "cpplocate C library")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "C++ Locator Library")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "opensource@cginternals.com")
set(CPACK_DEBIAN_PACKAGE_NAME "cpplocate")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_VERSION "2.3.0")
set(CPACK_DEB_COMPONENT_INSTALL "OFF")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "/Applications/CLion.app/Contents/bin/cmake/mac/share/cmake-3.21/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "cs5625 built using CMake")
set(CPACK_GENERATOR "TGZ")
set(CPACK_IGNORE_FILES "")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY "OFF")
set(CPACK_INSTALLED_DIRECTORIES "")
set(CPACK_INSTALL_CMAKE_PROJECTS "/Users/sze/Desktop/CS5625/Starter22/cmake-build-debug;cs5625;ALL;/")
set(CPACK_INSTALL_PREFIX "/usr/local")
set(CPACK_MODULE_PATH "/Users/sze/Desktop/CS5625/Starter22/ext/cpplocate/deploy/packages/cpplocate")
set(CPACK_NSIS_DISPLAY_NAME "cpplocate")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_NSIS_PACKAGE_NAME "cpplocate")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX12.0.sdk")
set(CPACK_OUTPUT_CONFIG_FILE "/Users/sze/Desktop/CS5625/Starter22/cmake-build-debug/ext/cpplocate/CPackConfig-cpplocate.cmake")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "/Users/sze/Desktop/CS5625/Starter22/ext/cpplocate/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ Locator Library")
set(CPACK_PACKAGE_FILE_NAME "cpplocate-2.3.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "cpplocate")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "cpplocate")
set(CPACK_PACKAGE_NAME "cpplocate")
set(CPACK_PACKAGE_RELOCATABLE "false")
set(CPACK_PACKAGE_VENDOR "CG Internals GmbH")
set(CPACK_PACKAGE_VERSION "2.3.0")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
set(CPACK_PATH "/Applications/CLion.app/Contents/bin/cmake/mac/bin")
set(CPACK_RESOURCE_FILE_LICENSE "/Users/sze/Desktop/CS5625/Starter22/ext/cpplocate/LICENSE")
set(CPACK_RESOURCE_FILE_README "/Users/sze/Desktop/CS5625/Starter22/ext/cpplocate/README.md")
set(CPACK_RESOURCE_FILE_WELCOME "/Users/sze/Desktop/CS5625/Starter22/ext/cpplocate/README.md")
set(CPACK_RPM_COMPONENT_INSTALL "OFF")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_DESCRIPTION "")
set(CPACK_RPM_PACKAGE_GROUP "unknown")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_NAME "cpplocate")
set(CPACK_RPM_PACKAGE_PROVIDES "")
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_RELOCATABLE "OFF")
set(CPACK_RPM_PACKAGE_REQUIRES "")
set(CPACK_RPM_PACKAGE_SUMMARY "C++ Locator Library")
set(CPACK_RPM_PACKAGE_VENDOR "CG Internals GmbH")
set(CPACK_RPM_PACKAGE_VERSION "2.3.0")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ;TXZ;TZ")
set(CPACK_SOURCE_IGNORE_FILES "")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/Users/sze/Desktop/CS5625/Starter22/cmake-build-debug/CPackSourceConfig.cmake")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "")
set(CPACK_SOURCE_RPM "OFF")
set(CPACK_SOURCE_TBZ2 "ON")
set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TOPLEVEL_TAG "")
set(CPACK_SOURCE_TXZ "ON")
set(CPACK_SOURCE_TZ "ON")
set(CPACK_SOURCE_ZIP "OFF")
set(CPACK_STRIP_FILES "")
set(CPACK_SYSTEM_NAME "Darwin")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "Darwin")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/Users/sze/Desktop/CS5625/Starter22/cmake-build-debug/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
cmake_minimum_required(VERSION 3.29)

set(KF_VERSION "6.26.0") # handled by release scripts
set(KF_DEP_VERSION "6.25.0") # handled by release scripts

project(KMime VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.25.0 NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMGenerateQDoc)
include(ECMCheckOutboundLicense)
include(ECMPoQmTools)
include(ECMFeatureSummary)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

find_package(KF6Codecs ${KF_DEP_VERSION} CONFIG REQUIRED)

set(REQUIRED_QT_VERSION 6.9.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core)

ecm_setup_version(PROJECT VARIABLE_PREFIX KMIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6MimeConfigVersion.cmake"
                        SOVERSION 6
)

option(BUILD_FUZZERS "Build KMime fuzzers" OFF)

########### Find packages ###########
find_package(KF6Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)
add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.11.0  KF 6.25.0)

add_subdirectory(src)
# TODO remove once the Gear master repos are ported and before the first KF release!
if (KF_DEP_VERSION VERSION_LESS 6.27)
    add_subdirectory(compat)
endif()
if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

if(BUILD_FUZZERS)
    add_subdirectory(autotests/ossfuzz)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Mime")

configure_package_config_file(
    "${KMime_SOURCE_DIR}/KF6MimeConfig.cmake.in"
    "${KMime_BINARY_DIR}/KF6MimeConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    )

install(FILES
    "${KMime_BINARY_DIR}/KF6MimeConfig.cmake"
    "${KMime_BINARY_DIR}/KF6MimeConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
    )

install(EXPORT KF6MimeTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KF6MimeTargets.cmake
    NAMESPACE KF6::
    )

install(FILES
    ${KMime_BINARY_DIR}/kmime_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KMime/
    COMPONENT Devel
    )

ecm_install_po_files_as_qm(poqm)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.h)
ecm_check_outbound_license(LICENSES LGPL-2.0-only LGPL-3.0-only FILES ${ALL_SOURCE_FILES})

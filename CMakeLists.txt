cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "6.5.3")

project(KMime VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "6.14.0")

find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${KMime_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)

include(ECMGenerateHeaders)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMFeatureSummary)
include(ECMAddQch)
include(ECMCheckOutboundLicense)
include(ECMPoQmTools)

set(KMIME_LIB_VERSION ${PIM_VERSION})
set(QT_REQUIRED_VERSION "6.7.0")


find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core)

ecm_setup_version(PROJECT VARIABLE_PREFIX KMIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6MimeConfigVersion.cmake"
                        SOVERSION 6
)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

########### Find packages ###########
find_package(KF6Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)
add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.10.0  KF 6.16.0)

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if(USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()

########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6Mime")
if(BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6Mime_QCH
        FILE KPim6MimeQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6MimeQchTargets.cmake\")")
endif()


configure_package_config_file(
    "${KMime_SOURCE_DIR}/KPimMimeConfig.cmake.in"
    "${KMime_BINARY_DIR}/KPim6MimeConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    )

install(FILES
    "${KMime_BINARY_DIR}/KPim6MimeConfig.cmake"
    "${KMime_BINARY_DIR}/KPim6MimeConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
    )

install(EXPORT KPim6MimeTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6MimeTargets.cmake
    NAMESPACE KPim6::
    )

install(FILES
    ${KMime_BINARY_DIR}/kmime_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KMime/
    COMPONENT Devel
    )

ecm_install_po_files_as_qm(poqm)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.h)
ecm_check_outbound_license(LICENSES LGPL-2.0-only LGPL-3.0-only FILES ${ALL_SOURCE_FILES})

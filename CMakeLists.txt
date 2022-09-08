cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.21.2")

project(KMime VERSION ${PIM_VERSION})

# ECM setup
set(KF5_MIN_VERSION "5.95.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${KMime_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGeneratePriFile)

include(ECMGenerateHeaders)
include(ECMSetupVersion)
include(FeatureSummary)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)

set(KMIME_LIB_VERSION ${PIM_VERSION})
set(QT_REQUIRED_VERSION "5.15.2")
find_package(Qt${QT_MAJOR_VERSION} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()

ecm_setup_version(PROJECT VARIABLE_PREFIX KMIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MimeConfigVersion.cmake"
                        SOVERSION 5
)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

########### Find packages ###########
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_MIN_VERSION} CONFIG REQUIRED)
add_definitions(-DTRANSLATION_DOMAIN=\"libkmime5\")
ecm_set_disabled_deprecation_versions(QT 5.15.2 KF 5.96.0)

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()
########### Targets ###########
add_subdirectory(src)
add_subdirectory(includes)

if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Mime")

configure_package_config_file(
    "${KMime_SOURCE_DIR}/KF5MimeConfig.cmake.in"
    "${KMime_BINARY_DIR}/KF5MimeConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    )

install(FILES
    "${KMime_BINARY_DIR}/KF5MimeConfig.cmake"
    "${KMime_BINARY_DIR}/KF5MimeConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
    )

install(EXPORT KF5MimeTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KF5MimeTargets.cmake
    NAMESPACE KF5::
    )

install(FILES
    ${KMime_BINARY_DIR}/kmime_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KMime/
    COMPONENT Devel
    )

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Mime_QCH
        FILE KF5MimeQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5MimeQchTargets.cmake\")")
endif()


ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

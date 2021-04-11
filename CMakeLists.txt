cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.17.40")

project(KMime VERSION ${PIM_VERSION})

# ECM setup
set(KF5_MIN_VERSION "5.81.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${KMime_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGeneratePriFile)

include(ECMGenerateHeaders)
include(ECMSetupVersion)
include(FeatureSummary)
include(ECMQtDeclareLoggingCategory)


set(KMIME_LIB_VERSION ${PIM_VERSION})
set(QT_REQUIRED_VERSION "5.15.0")
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core)

ecm_setup_version(PROJECT VARIABLE_PREFIX KMIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MimeConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_MIN_VERSION} CONFIG REQUIRED)
add_definitions(-DTRANSLATION_DOMAIN=\"libkmime5\")
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055200)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}
    COMPONENT Devel
    )

ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

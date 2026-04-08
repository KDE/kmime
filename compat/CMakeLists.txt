# TODO temporary until Gear consumers in master are ported to link to KF6::Mime instead
add_library(KPim6Mime)
add_library(KPim6::Mime ALIAS KPim6Mime)

set_target_properties(KPim6Mime PROPERTIES
    VERSION ${KMIME_VERSION}
    SOVERSION ${KMIME_SOVERSION}
    EXPORT_NAME Mime
)
target_link_libraries(KPim6Mime PUBLIC KF6Mime)

install(TARGETS KPim6Mime EXPORT KPim6MimeTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6Mime")

ecm_setup_version(PROJECT VARIABLE_PREFIX KMIME
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6MimeConfigVersion.cmake"
                  SOVERSION 6
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPim6MimeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6MimeConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6MimeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6MimeConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

install(EXPORT KPim6MimeTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6MimeTargets.cmake
    NAMESPACE KPim6::
)

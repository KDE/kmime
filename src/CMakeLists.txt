include(ConfigureChecks)

configure_file(config-kmime.h.in ${KMime_BINARY_DIR}/src/config-kmime.h)

# Turn exceptions on
kde_enable_exceptions()

########### next target ###############
add_library(KF6Mime)
add_library(KF6::Mime ALIAS KF6Mime)

ecm_create_qm_loader(KF6Mime libkmime6_qt)

target_sources(KF6Mime PRIVATE
   charfreq.cpp
   util.cpp
   util_p.cpp
   mdn.cpp
   parsers.cpp
   headerparsing.cpp
   headerfactory.cpp
   content.cpp
   contentindex.cpp
   headers.cpp
   message.cpp
   newsarticle.cpp
   codecs.cpp
   types.cpp

   charfreq_p.h
   util.h
   mdn.h
   parsers_p.h
   headerparsing.h
   headerparsing_p.h
   headerfactory_p.h
   content.h
   contentindex.h
   headers.h
   message.h
   newsarticle.h
   codecs_p.h
   types.h
)

ecm_qt_declare_logging_category(KF6Mime
       HEADER kmime_debug.h
       IDENTIFIER KMIME_LOG
       CATEGORY_NAME org.kde.pim.kmime
        DESCRIPTION "kmime (pim lib)" EXPORT KMIME)

ecm_generate_export_header(KF6Mime
    BASE_NAME KMime
    VERSION ${KMIME_VERSION}
    DEPRECATED_BASE_VERSION 0
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.7
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Mime INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KMime>")
target_include_directories(KF6Mime PUBLIC "$<BUILD_INTERFACE:${KMime_SOURCE_DIR}/src;${KMime_BINARY_DIR}/src;${KMime_BINARY_DIR}>")
target_compile_features(KF6Mime PUBLIC cxx_std_20)

set(_kmime_extra_libs)
if(WIN32)
    set(_kmime_extra_libs ws2_32)
endif()

target_link_libraries(KF6Mime
PUBLIC
    Qt::Core
PRIVATE
    KF6::Codecs
    ${_kmime_extra_libs}
)

set_target_properties(KF6Mime PROPERTIES
    VERSION ${KMIME_VERSION}
    SOVERSION ${KMIME_SOVERSION}
    EXPORT_NAME Mime
)

install(TARGETS KF6Mime EXPORT KF6MimeTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KMime_CamelCase_HEADERS
  HEADER_NAMES
      Content
      ContentIndex
      Headers
      Message
      Util
      HeaderParsing
      Types
      MDN
  REQUIRED_HEADERS KMime_HEADERS
  PREFIX KMime
)

install(FILES
    ${KMime_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KF6/KMime/KMime
)

install(FILES
     ${KMime_BINARY_DIR}/src/kmime_export.h
     ${KMime_HEADERS}
     DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KF6/KMime/kmime
     COMPONENT Devel
)

ecm_qt_install_logging_categories(EXPORT KMIME FILE kmime.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
ecm_generate_qdoc(KF6Mime kmime.qdocconf)

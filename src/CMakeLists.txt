include(ConfigureChecks)

configure_file(config-kmime.h.cmake ${KMime_BINARY_DIR}/src/config-kmime.h)

# Turn exceptions on
kde_enable_exceptions()

########### next target ###############
add_library(KF5Mime)
add_library(KF5::Mime ALIAS KF5Mime)

target_sources(KF5Mime PRIVATE
   kmime_charfreq.cpp
   kmime_util.cpp
   kmime_mdn.cpp
   kmime_parsers.cpp
   kmime_header_parsing.cpp
   kmime_headerfactory.cpp
   kmime_content.cpp
   kmime_contentindex.cpp
   kmime_headers.cpp
   kmime_message.cpp
   kmime_newsarticle.cpp
   kmime_dateformatter.cpp
   kmime_codecs.cpp
   kmime_types.cpp
)

ecm_qt_declare_logging_category(KF5Mime
       HEADER kmime_debug.h
       IDENTIFIER KMIME_LOG
       CATEGORY_NAME org.kde.pim.kmime
        DESCRIPTION "kmime (pim lib)" EXPORT KMIME)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5Mime PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(KF5Mime BASE_NAME KMime)

target_include_directories(KF5Mime INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KMime>")
target_include_directories(KF5Mime PUBLIC "$<BUILD_INTERFACE:${KMime_SOURCE_DIR}/src;${KMime_BINARY_DIR}/src;${KMime_BINARY_DIR}>")

if (WIN32)
    set(_kmime_extra_libs ws2_32)
endif()

target_link_libraries(KF5Mime
PUBLIC
    Qt::Core
PRIVATE
    KF5::I18n
    KF5::Codecs
    ${_kmime_extra_libs}
)

set_target_properties(KF5Mime PROPERTIES
    VERSION ${KMIME_VERSION}
    SOVERSION ${KMIME_SOVERSION}
    EXPORT_NAME Mime
)

install(TARGETS KF5Mime EXPORT KF5MimeTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

# Don't install them but needed to compile kdepimlibs
ecm_generate_headers(KMime_CamelCase_HEADERS
  HEADER_NAMES
  KMime_Content
  KMime_Message
  KMime_Util
  KMime_Dateformatter
  KMime_Header_Parsing
  PREFIX KMime

  REQUIRED_HEADERS KMime_HEADERS
)


install(FILES
         ${KMime_BINARY_DIR}/src/kmime_export.h
         kmime_content.h
         kmime_contentindex.h
         kmime_header_parsing.h
         kmime_headers.h
         kmime_message.h
         kmime_mdn.h
         kmime_newsarticle.h
         kmime_dateformatter.h
         kmime_util.h
         kmime_types.h
         DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KMime/kmime COMPONENT Devel
)

ecm_qt_install_logging_categories(EXPORT KMIME FILE kmime.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

ecm_generate_pri_file(BASE_NAME KMime LIB_NAME KF5Mime DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KMime)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

include(ConfigureChecks)

configure_file(config-kmime.h.in ${KMime_BINARY_DIR}/src/config-kmime.h)

# Turn exceptions on
kde_enable_exceptions()

########### next target ###############
add_library(KPim6Mime)
add_library(KPim6::Mime ALIAS KPim6Mime)

ecm_create_qm_loader(KPim6Mime libkmime6_qt)

target_sources(KPim6Mime PRIVATE
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

ecm_qt_declare_logging_category(KPim6Mime
       HEADER kmime_debug.h
       IDENTIFIER KMIME_LOG
       CATEGORY_NAME org.kde.pim.kmime
        DESCRIPTION "kmime (pim lib)" EXPORT KMIME)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6Mime PROPERTIES UNITY_BUILD ON)
endif()
ecm_generate_export_header(KPim6Mime
    BASE_NAME KMime
    VERSION ${KMIME_VERSION}
    DEPRECATED_BASE_VERSION 0
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.7
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}    
)

target_include_directories(KPim6Mime INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/KMime>")
target_include_directories(KPim6Mime PUBLIC "$<BUILD_INTERFACE:${KMime_SOURCE_DIR}/src;${KMime_BINARY_DIR}/src;${KMime_BINARY_DIR}>")
target_compile_features(KPim6Mime PUBLIC cxx_std_20)

if(WIN32)
    set(_kmime_extra_libs ws2_32)
endif()

target_link_libraries(KPim6Mime
PUBLIC
    Qt::Core
PRIVATE
    KF6::Codecs
    ${_kmime_extra_libs}
)

set_target_properties(KPim6Mime PROPERTIES
    VERSION ${KMIME_VERSION}
    SOVERSION ${KMIME_SOVERSION}
    EXPORT_NAME Mime
)

install(TARGETS KPim6Mime EXPORT KPim6MimeTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

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
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KMime/KMime
)

install(FILES
     ${KMime_BINARY_DIR}/src/kmime_export.h
     ${KMime_HEADERS}
     DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KMime/kmime
     COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KPim6Mime_QCH
        NAME KMime
        BASE_NAME KPim6Mime
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KMime_CamelCase_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KMIME_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

ecm_qt_install_logging_categories(EXPORT KMIME FILE kmime.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

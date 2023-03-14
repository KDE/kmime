include(ConfigureChecks)

configure_file(config-kmime.h.cmake ${KMime_BINARY_DIR}/src/config-kmime.h)

# Turn exceptions on
kde_enable_exceptions()

########### next target ###############
add_library(KPim${KF_MAJOR_VERSION}Mime)
add_library(KPim${KF_MAJOR_VERSION}::Mime ALIAS KPim${KF_MAJOR_VERSION}Mime)

target_sources(KPim${KF_MAJOR_VERSION}Mime PRIVATE
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

   kmime_charfreq.h
   kmime_util.h
   kmime_mdn.h
   kmime_parsers.h
   kmime_header_parsing.h
   kmime_headerfactory_p.h
   kmime_content.h
   kmime_contentindex.h
   kmime_headers.h
   kmime_message.h
   kmime_newsarticle.h
   kmime_dateformatter.h
   kmime_codecs.h
   kmime_types.h
   )

ecm_qt_declare_logging_category(KPim${KF_MAJOR_VERSION}Mime
       HEADER kmime_debug.h
       IDENTIFIER KMIME_LOG
       CATEGORY_NAME org.kde.pim.kmime
        DESCRIPTION "kmime (pim lib)" EXPORT KMIME)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}Mime PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(KPim${KF_MAJOR_VERSION}Mime BASE_NAME KMime)

target_include_directories(KPim${KF_MAJOR_VERSION}Mime INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KMime>")
target_include_directories(KPim${KF_MAJOR_VERSION}Mime PUBLIC "$<BUILD_INTERFACE:${KMime_SOURCE_DIR}/src;${KMime_BINARY_DIR}/src;${KMime_BINARY_DIR}>")

if (WIN32)
    set(_kmime_extra_libs ws2_32)
endif()

target_link_libraries(KPim${KF_MAJOR_VERSION}Mime
PUBLIC
    Qt::Core
PRIVATE
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Codecs
    ${_kmime_extra_libs}
)
if (TARGET Qt6::Core5Compat)
    target_link_libraries(KPim${KF_MAJOR_VERSION}Mime PRIVATE Qt6::Core5Compat) # QTextCodec
endif()

set_target_properties(KPim${KF_MAJOR_VERSION}Mime PROPERTIES
    VERSION ${KMIME_VERSION}
    SOVERSION ${KMIME_SOVERSION}
    EXPORT_NAME Mime
)

install(TARGETS KPim${KF_MAJOR_VERSION}Mime EXPORT KPim${KF_MAJOR_VERSION}MimeTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

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
         DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KMime/kmime COMPONENT Devel
)

if (BUILD_QCH)
    ecm_add_qch(
        KPim${KF_MAJOR_VERSION}Mime_QCH
        NAME KMime
        BASE_NAME KPim${KF_MAJOR_VERSION}Mime
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KMime_CamelCase_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt${QT_MAJOR_VERSION}Core_QCH
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

ecm_generate_pri_file(BASE_NAME KMime LIB_NAME KPim${KF_MAJOR_VERSION}Mime DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KMime)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

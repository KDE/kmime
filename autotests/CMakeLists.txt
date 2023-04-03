include(ECMMarkAsTest)

find_package(Qt6Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

# Turn exceptions on
kde_enable_exceptions()

ecm_qt_declare_logging_category(kmime_autotest_SRCS
   HEADER kmime_debug.h
   IDENTIFIER KMIME_LOG
   CATEGORY_NAME org.kde.pim.kmime
)


macro(ADD_KMIME_TEST)
  foreach(_testName ${ARGN})
    add_executable(${_testName} ${_testName}.cpp ${kmime_autotest_SRCS})
    add_test(NAME kmime-${_testName} COMMAND ${_testName})
    target_link_libraries(${_testName}
      KPim6Mime
      Qt::Test
      KF6::Codecs
    )
    if (TARGET Qt6::Core5Compat)
        target_link_libraries(${_testName} Qt6::Core5Compat) # QTextCodec
    endif()
    set_target_properties(${_testName} PROPERTIES COMPILE_FLAGS -DTEST_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"" )
    ecm_mark_as_test(${_testName})
    set_tests_properties(kmime-${_testName} PROPERTIES ENVIRONMENT "LANG=C")
  endforeach()
endmacro()

add_kmime_test(
  rfc2047test
  utiltest
  contentindextest
  charfreqtest
  headertest
  contenttest
  messagetest
  headerfactorytest
  rfc2231test
  sizetest
  parsedatetimetest
  dateformattertest
  attachmenttest
  typestest
)

include(CheckIncludeFile)
check_include_file(getopt.h HAVE_GETOPT_H)

if(HAVE_GETOPT_H)

  add_executable(test_mdn test_mdn.cpp)
  add_test(NAME test_mdn COMMAND test_mdn)
  target_link_libraries(test_mdn KPim6Mime)

endif()

########### next target ###############

add_executable(test_charfreq test_charfreq.cpp)
add_test(NAME test_charfreq COMMAND test_charfreq)
target_link_libraries(test_charfreq KPim6Mime)

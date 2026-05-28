add_executable(test_charfreq test_charfreq.cpp)
add_test(NAME test_charfreq COMMAND test_charfreq)
target_link_libraries(test_charfreq KF6Mime)

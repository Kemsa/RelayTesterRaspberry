add_executable(relaytester_unit_tests
  main.cpp
  test_contactselector.cpp
  test_dynamicreadings.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../../src/controls/contactselector.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../../src/controls/dynamicreadings.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../../src/controls/dynamicswitch.cpp
)

target_include_directories(relaytester_unit_tests
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../src/controls
    ${CMAKE_CURRENT_LIST_DIR}/../../libs/Wiringpi_Wrapper/lib/include
)

target_link_libraries(relaytester_unit_tests
  PRIVATE
    wiringpi_wrapper
    Qt5::Core
    Qt5::Test
)

if(WIN32)
  get_filename_component(_mingw_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)

  add_custom_command(TARGET relaytester_unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:wiringpi_wrapper>
      $<TARGET_FILE_DIR:relaytester_unit_tests>
    COMMENT "Deploying wiringpi_wrapper runtime dependency for relaytester_unit_tests..."
  )

  add_custom_command(TARGET relaytester_unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${_mingw_bin_dir}/libstdc++-6.dll"
      "${_mingw_bin_dir}/libgcc_s_seh-1.dll"
      "${_mingw_bin_dir}/libwinpthread-1.dll"
      $<TARGET_FILE_DIR:relaytester_unit_tests>
    COMMENT "Deploying MinGW runtime dependencies for relaytester_unit_tests..."
  )

  add_custom_command(TARGET relaytester_unit_tests POST_BUILD
    COMMAND ${Qt5_DIR}/../../../bin/windeployqt.exe
    ARGS --debug --force $<TARGET_FILE:relaytester_unit_tests>
    COMMENT "Deploying Qt runtime dependencies for relaytester_unit_tests..."
  )
endif()

add_test(NAME ContactSelectorUnitTest COMMAND relaytester_unit_tests)

cmake_minimum_required(VERSION 3.16)

project(relayTester VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Try to add Picolog submodule; it's optional if SDK is not available
add_subdirectory(libs/Picolog_lib)

# Add WiringPi wrapper library.
add_subdirectory(libs/Wiringpi_Wrapper)

qt5_wrap_ui(UI_HEADERS mainwindow.ui)
add_executable(relayTester
  main.cpp
  mainwindow.cpp
  mainwindow.h
  ${UI_HEADERS}
)
target_include_directories(relayTester PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/Picolog_lib/src
)

set(RELAYTESTER_LIBS
  Qt5::Widgets
  wiringpi_wrapper
  picolog24)

target_link_libraries(relayTester PRIVATE ${RELAYTESTER_LIBS})

# Deploy Qt runtime dependencies on Windows
if(WIN32)
  if(TARGET wiringpi_wrapper)
    add_custom_command(TARGET relayTester POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:wiringpi_wrapper>
        $<TARGET_FILE_DIR:relayTester>
      COMMENT "Deploying wiringpi_wrapper runtime library..."
    )
  endif()

  add_custom_command(TARGET relayTester POST_BUILD
    COMMAND ${Qt5_DIR}/../../../bin/windeployqt.exe
    ARGS --debug --force $<TARGET_FILE:relayTester>
    COMMENT "Deploying Qt5 runtime dependencies..."
  )
  
  # Deploy Pico HRDL DLL if available
  find_file(PICO_SDK_HRDL_DLL
    NAMES PicoHRDL.dll picohrdl.dll
    PATHS
      "C:/Program Files/Pico Technology/SDK/bin"
      "C:/Program Files/Pico Technology/SDK/lib"
    NO_DEFAULT_PATH
  )
  
  if(PICO_SDK_HRDL_DLL)
    add_custom_command(TARGET relayTester POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PICO_SDK_HRDL_DLL}"
        "$<TARGET_FILE_DIR:relayTester>/PicoHRDL.dll"
      COMMENT "Deploying PicoHRDL.dll..."
    )
  endif()
endif()

set_target_properties(relayTester PROPERTIES
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS relayTester
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

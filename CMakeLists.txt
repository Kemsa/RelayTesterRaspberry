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


qt5_wrap_ui(UI_HEADERS src/widgets/mainWindow.ui)
add_executable(relayTester
  ${UI_HEADERS}
  main.cpp src/widgets/mainWindow.cpp src/widgets/mainWindow.h
  src/widgets/homescreen.h src/widgets/homescreen.cpp src/widgets/homescreen.ui
  src/widgets/calibration.h src/widgets/calibration.cpp src/widgets/calibration.ui
  src/controls/navigator.h src/controls/navigator.cpp

)
target_include_directories(relayTester PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/Picolog_lib/src
  src
  src/widgets
  src/controls
)

set(RELAYTESTER_LIBS
  Qt5::Widgets
  wiringpi_wrapper
  picolog24)

target_link_libraries(relayTester PRIVATE ${RELAYTESTER_LIBS})

set(RELAYTESTER_ASSETS
  styles.css
  colors.csv
)

add_custom_target(copy_relaytester_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${RELAYTESTER_ASSETS}
    $<TARGET_FILE_DIR:relayTester>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Copying relayTester style assets..."
)

# Deploy library runtime dependencies on Windows
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

install(FILES ${RELAYTESTER_ASSETS}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

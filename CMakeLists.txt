cmake_minimum_required(VERSION 3.16)

project(relayTester VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RELAYTESTER_BUILD_UNIT_TESTS "Build relayTester unit-test targets" OFF)

find_package(Qt5 COMPONENTS Widgets SerialPort REQUIRED)

# Try to add Picolog submodule; it's optional if SDK is not available
add_subdirectory(libs/Picolog_lib)

# Add WiringPi wrapper library.
add_subdirectory(libs/Wiringpi_Wrapper)

# Add the power supply library with a Windows-safe mock backend.
add_subdirectory(libs/PowerSupply)

if(RELAYTESTER_BUILD_UNIT_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(unit-tests)
endif()


qt5_wrap_ui(UI_HEADERS src/UI/screens/mainWindow.ui)
add_executable(relayTester
  ${UI_HEADERS}
  main.cpp src/UI/screens/mainWindow.cpp src/UI/screens/mainWindow.h
  src/controls/logbus.cpp src/controls/logbus.h
  src/UI/screens/homescreen.h src/UI/screens/homescreen.cpp src/UI/screens/homescreen.ui
  src/UI/screens/calibrationscreen.h src/UI/screens/calibrationscreen.cpp src/UI/screens/calibrationscreen.ui
  src/controls/navigator.h src/controls/navigator.cpp
  src/UI/widgets/contactselectwidget.h src/UI/widgets/contactselectwidget.cpp src/UI/widgets/contactselectwidget.ui
  src/controls/contactselector.h src/controls/contactselector.cpp
  config.h
  src/UI/widgets/powersupplysetupwidget.h src/UI/widgets/powersupplysetupwidget.cpp src/UI/widgets/powersupplysetupwidget.ui
  src/UI/widgets/led.h src/UI/widgets/led.cpp
  src/UI/widgets/adc24reading.h src/UI/widgets/adc24reading.cpp src/UI/widgets/adc24reading.ui
  src/controls/staticreadings.cpp src/controls/staticreadings.h
  src/controls/dynamicreadings.h src/controls/dynamicreadings.cpp
  src/controls/coilcontrol.h src/controls/coilcontrol.cpp
  src/controls/dynamicswitch.h src/controls/dynamicswitch.cpp

)
target_include_directories(relayTester PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/Picolog_lib/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/screens
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/src/controls
)

set(RELAYTESTER_LIBS
  Qt5::Widgets
  Qt5::SerialPort
  wiringpi_wrapper
  picolog24
  PowerSupplyLib)

target_link_libraries(relayTester PRIVATE ${RELAYTESTER_LIBS})

if(UNIX AND NOT APPLE)
  set_target_properties(relayTester PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
  )

  if(TARGET wiringpi_wrapper)
    set_target_properties(wiringpi_wrapper PROPERTIES
      BUILD_RPATH "$ORIGIN"
      INSTALL_RPATH "$ORIGIN"
    )
  endif()
endif()

set(RELAYTESTER_ASSETS
  src/UI/styles.css
  src/UI/colors.csv
)

add_custom_command(TARGET relayTester POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:relayTester>/UI
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${RELAYTESTER_ASSETS}
    $<TARGET_FILE_DIR:relayTester>/UI
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
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${Qt5_DIR}/../../../bin/Qt5SerialPort.dll"
      "$<TARGET_FILE_DIR:relayTester>"
    COMMENT "Deploying Qt5SerialPort.dll..."
  )
  
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

if(WIN32)
  # Show a console in Debug for qDebug/stdout visibility, keep GUI subsystem otherwise.
  set_target_properties(relayTester PROPERTIES
      WIN32_EXECUTABLE $<$<CONFIG:Debug>:FALSE>$<$<NOT:$<CONFIG:Debug>>:TRUE>
  )
endif()

include(GNUInstallDirs)
install(TARGETS relayTester
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(TARGET wiringpi_wrapper)
  install(TARGETS wiringpi_wrapper
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

install(FILES ${RELAYTESTER_ASSETS}
  DESTINATION ${CMAKE_INSTALL_BINDIR}/UI
)

if(UNIX AND NOT APPLE)
  install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR resolved_deps
      UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
      EXECUTABLES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/relayTester"
      DIRECTORIES
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
        "/opt/picoscope/lib"
        "/usr/lib"
        "/usr/lib/arm-linux-gnueabihf"
        "/lib/arm-linux-gnueabihf"
        "/opt/rpi-toolchain/sysroot/usr/lib/arm-linux-gnueabihf"
        "/opt/rpi-toolchain/sysroot/lib/arm-linux-gnueabihf"
        "/opt/rpi-toolchain/sysroot/usr/lib"
        "/opt/rpi-toolchain/sysroot/lib"
      PRE_EXCLUDE_REGEXES
        "linux-vdso.*"
        "ld-linux.*"
    )

    if(unresolved_deps)
      message(WARNING "Unresolved runtime dependencies: ${unresolved_deps}")
    endif()

    foreach(dep IN LISTS resolved_deps)
      file(INSTALL
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
        TYPE SHARED_LIBRARY
        FOLLOW_SYMLINK_CHAIN
        FILES "${dep}"
      )
    endforeach()
  ]])
endif()

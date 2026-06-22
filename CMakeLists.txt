cmake_minimum_required(VERSION 3.16)

project(relayTester VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Qt5 by default to keep runtime compatibility predictable on Windows.
option(RELAYTESTER_USE_QT6 "Build relayTester with Qt6" OFF)

if(RELAYTESTER_USE_QT6)
  find_package(Qt6 COMPONENTS Widgets REQUIRED)
else()
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

if(RELAYTESTER_USE_QT6)
  qt_standard_project_setup()
  qt_add_executable(relayTester
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
  )
  target_link_libraries(relayTester PRIVATE Qt6::Widgets)
else()
  qt5_wrap_ui(UI_HEADERS mainwindow.ui)
  add_executable(relayTester
    main.cpp
    mainwindow.cpp
    mainwindow.h
    ${UI_HEADERS}
  )
  target_include_directories(relayTester PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(relayTester PRIVATE Qt5::Widgets)
endif()

# Deploy Qt runtime dependencies on Windows
if(WIN32 AND NOT RELAYTESTER_USE_QT6)
  add_custom_command(TARGET relayTester POST_BUILD
    COMMAND ${Qt5_DIR}/../../../bin/windeployqt.exe
    ARGS --debug --force $<TARGET_FILE:relayTester>
    COMMENT "Deploying Qt5 runtime dependencies..."
  )
elseif(WIN32 AND RELAYTESTER_USE_QT6)
  add_custom_command(TARGET relayTester POST_BUILD
    COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe
    ARGS --debug --force $<TARGET_FILE:relayTester>
    COMMENT "Deploying Qt6 runtime dependencies..."
  )
endif()

set_target_properties(relayTester PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS relayTester
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

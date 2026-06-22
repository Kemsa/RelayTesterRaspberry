cmake_minimum_required(VERSION 3.16)

project(relayTester VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

qt5_wrap_ui(UI_HEADERS mainwindow.ui)
add_executable(relayTester
  main.cpp
  mainwindow.cpp
  mainwindow.h
  ${UI_HEADERS}
)
target_include_directories(relayTester PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(relayTester PRIVATE Qt5::Widgets)

# Deploy Qt runtime dependencies on Windows
if(WIN32)
  add_custom_command(TARGET relayTester POST_BUILD
    COMMAND ${Qt5_DIR}/../../../bin/windeployqt.exe
    ARGS --debug --force $<TARGET_FILE:relayTester>
    COMMENT "Deploying Qt5 runtime dependencies..."
  )
endif()

set_target_properties(relayTester PROPERTIES
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS relayTester
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

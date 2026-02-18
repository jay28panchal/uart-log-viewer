cmake_minimum_required(VERSION 3.20)
project(uart_log_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/serialtab.cpp
    src/finddialog.cpp
    src/timezonedialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/serialtab.h
    src/finddialog.h
    src/timezonedialog.h
)

qt_add_executable(uart-log-viewer
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(uart-log-viewer PRIVATE Qt6::Widgets Qt6::SerialPort)

set_target_properties(uart-log-viewer PROPERTIES
    MACOSX_BUNDLE TRUE
)

install(TARGETS uart-log-viewer
    RUNTIME DESTINATION bin
)

include(CPack)
set(CPACK_PACKAGE_NAME "uart-log-viewer")
file(READ "${CMAKE_SOURCE_DIR}/../VERSION" VERSION_FILE)
string(STRIP "${VERSION_FILE}" APP_VERSION)
set(CPACK_PACKAGE_VERSION "${APP_VERSION}")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "UART Log Viewer")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6, libqt6serialport6, libqt6core6, libqt6gui6")
set(CPACK_PACKAGE_FILE_NAME "uart-log-viewer_${APP_VERSION}_amd64")

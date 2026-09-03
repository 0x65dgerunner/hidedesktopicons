cmake_minimum_required(VERSION 3.16)

project(HideDesktopIcons VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_PREFIX_PATH)
    if(EXISTS "C:/Qt/6.8.2/msvc2022_64/lib/cmake/Qt6")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.8.2/msvc2022_64")
    elseif(EXISTS "C:/Qt/6.7.3/msvc2022_64/lib/cmake/Qt6")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2022_64")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

option(BUILD_LITE "Build the Win32 tray/invisible executable" ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(FLUENT_QT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FLUENT_QT_BUILD_GALLERY OFF CACHE BOOL "" FORCE)
set(FLUENT_QT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FLUENT_QT_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/Fluent-Qt)

# MSVC rcc mishandles non-ASCII absolute paths; use relative paths in the generated qrc.
set(_fluent_qrc "${CMAKE_BINARY_DIR}/third_party/Fluent-Qt/generated/resources.qrc")
if(EXISTS "${_fluent_qrc}")
    file(READ "${_fluent_qrc}" _fluent_qrc_contents)
    string(REPLACE "${CMAKE_SOURCE_DIR}/third_party/Fluent-Qt"
           "../../../../third_party/Fluent-Qt" _fluent_qrc_contents "${_fluent_qrc_contents}")
    file(WRITE "${_fluent_qrc}" "${_fluent_qrc_contents}")
endif()
unset(_fluent_qrc)
unset(_fluent_qrc_contents)

set(HIDEICONS_CORE_SOURCES
    src/json_utils.cpp
    src/i18n.cpp
    src/settings.cpp
    src/desktop_icons.cpp
    src/hotkey.cpp
    src/startup.cpp
)

add_library(hideicons_core STATIC ${HIDEICONS_CORE_SOURCES})
target_include_directories(hideicons_core PUBLIC src)
if(WIN32)
    target_link_libraries(hideicons_core PUBLIC user32 shell32 advapi32)
    target_compile_definitions(hideicons_core PUBLIC _WIN32_WINNT=0x0A00 UNICODE _UNICODE NOMINMAX)
endif()

add_executable(HideDesktopIcons WIN32
    gui/main.cpp
    gui/mainwindow.cpp
    gui/mainwindow.hpp
    gui/icon_helper.cpp
    gui/icon_helper.hpp
    gui/resources.qrc
)

target_link_libraries(HideDesktopIcons PRIVATE
    hideicons_core
    FluentQt::FluentQt
    Qt6::Widgets
    Qt6::Svg
)

if(WIN32)
    target_compile_definitions(HideDesktopIcons PRIVATE UNICODE _UNICODE NOMINMAX)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET HideDesktopIcons POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-compiler-runtime "$<TARGET_FILE:HideDesktopIcons>"
            COMMENT "Deploying Qt runtime for HideDesktopIcons"
        )
    endif()
    add_custom_command(TARGET HideDesktopIcons POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/locales"
            "$<TARGET_FILE_DIR:HideDesktopIcons>/locales"
        COMMENT "Copying locale dictionaries"
    )
endif()

if(BUILD_LITE AND WIN32)
    add_executable(HideDesktopIconsLite WIN32 lite/main.cpp)
    target_link_libraries(HideDesktopIconsLite PRIVATE hideicons_core)
    target_compile_definitions(HideDesktopIconsLite PRIVATE UNICODE _UNICODE NOMINMAX)
endif()

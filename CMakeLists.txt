cmake_minimum_required(VERSION 3.16)

project(CKTextEditor VERSION 0.8 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network LinguistTools)
qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE cn
    I18N_TRANSLATED_LANGUAGES en
)

add_subdirectory(cktext)

add_definitions(
    -DPRJ_ARCH="${CMAKE_SYSTEM_PROCESSOR}"
    -DPRJ_VERSION="${PROJECT_VERSION}"
    -DQT_VERSION="${QT_VERSION}"
    -DPRJ_LICENCE="MIT"
    -DPRJ_REPOS="https://gitee.com/chenke404/cktext-editor|https://github.com/ChenKe404/CKTextEditor.git"
)

file(GLOB_RECURSE SOURCES
    dialog/*.h dialog/*.cpp dialog/*.ui
    control/*.h control/*.cpp
)

add_executable(CKTextEditor
    res.qrc
    main.cpp
    pch.h pch.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    highlighter.h highlighter.cpp
    ${SOURCES}
)

target_precompile_headers(CKTextEditor PRIVATE pch.h)
target_include_directories(CKTextEditor PRIVATE
    . dialog
)
target_link_libraries(CKTextEditor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    cktext
)

add_subdirectory(plugin)

if(WIN32)
    set_target_properties(CKTextEditor PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()

# Translations
qt_add_translations(CKTextEditor
    TS_FILES
        translations/cktexteditor_en_us.ts   # 美式英语翻译文件
    LUPDATE_TARGET tr_update
    LRELEASE_TARGET tr_release
)
add_dependencies(CKTextEditor tr_update tr_release)


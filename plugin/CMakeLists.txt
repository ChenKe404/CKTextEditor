
set(3RD_ROOT  ${CMAKE_CURRENT_SOURCE_DIR}/3rd)

add_library(plugin STATIC plugin.h)
target_include_directories(plugin PUBLIC
    .
)
target_link_libraries(plugin PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_subdirectory(${3RD_ROOT}/libpe++)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(import)

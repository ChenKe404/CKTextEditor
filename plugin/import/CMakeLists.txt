
set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
add_library(im_tiberian_sun SHARED
    im_tiberian_sun.cpp
    dlg_im_ts.h dlg_im_ts.cpp dlg_im_ts.ui
)
target_include_directories(im_tiberian_sun
    PRIVATE
    ${3RD_ROOT}/include
    ${LIBPE_DIR}/include
)
target_link_libraries(im_tiberian_sun PRIVATE
    plugin
    cktext
    libpe++
    Qt6::Core5Compat
)



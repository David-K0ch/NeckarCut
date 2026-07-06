cmake_minimum_required(VERSION 3.23)

project(NeckarCut)


add_library(NeckarCutDataStructure STATIC)
target_sources(NeckarCutDataStructure
    PRIVATE
        impl/objectPool.cc
        impl/myTree.cc
        impl/myForest.cc
    PUBLIC
        FILE_SET HEADERS
        FILES
            src/myArray.hh
            src/objectPool.hh
            src/myTree.hh
            src/myForest.hh
            src/myQueue.hh
            src/global.hh
            src/macros.hh
)
target_compile_options(NeckarCutDataStructure
    PRIVATE
        -O3
        -flto
        -march=native
)

target_include_directories(NeckarCutDataStructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


add_executable(NeckarCut)
target_sources(NeckarCut
    PRIVATE
        NeckarCut.cpp
)
target_compile_options(NeckarCut
    PRIVATE
        -O3
        -flto
        -march=native
)

target_link_options(NeckarCut PRIVATE
    -static
    -flto
)

target_link_libraries(NeckarCut
    PRIVATE
        NeckarCutDataStructure
)











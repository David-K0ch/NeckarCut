cmake_minimum_required(VERSION 3.23)

project(Pace26)


add_library(myDataStructure STATIC)
target_sources(myDataStructure
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
target_compile_options(myDataStructure
    PRIVATE
        -O3
        -flto
)

target_include_directories(myDataStructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


add_executable(mySolution)
target_sources(mySolution
    PRIVATE
        mySolution.cc
)
target_compile_options(mySolution
    PRIVATE
        -O3
        -flto
)

target_link_options(mySolution PRIVATE
    -static
    -flto
)

target_link_libraries(mySolution
    PRIVATE
        myDataStructure
)














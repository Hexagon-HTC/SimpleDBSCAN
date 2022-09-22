cmake_minimum_required(VERSION 3.14.3)

project(dbscan)

add_library(dbscan STATIC
    include/dbscan.h
    include/kdtree.h
    src/kdtree.cpp)

target_include_directories(dbscan PUBLIC include)


set_target_properties( dbscan
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

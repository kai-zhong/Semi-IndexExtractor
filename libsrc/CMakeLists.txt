# add graph lib source code
file(GLOB UTIL_FILE ${PROJECT_SOURCE_DIR}/libsrc/util/*.cpp)
message(STATUS "UTIL_SOURCES: " ${UTIL_FILE})
add_library(util STATIC  ${UTIL_FILE})

file(GLOB GRAPH_FILE ${PROJECT_SOURCE_DIR}/libsrc/graph/*.cpp)
message(STATUS "GRAPH_SOURCES: " ${GRAPH_FILE})
add_library(graph STATIC ${GRAPH_FILE})

file(GLOB MBPTREE_FILE ${PROJECT_SOURCE_DIR}/libsrc/mbptree/*.cpp)
message(STATUS "MBPTREE_SOURCES: " ${MBPTREE_FILE})
add_library(mbptree STATIC  ${MBPTREE_FILE})

file(GLOB SEMIINDEXEXTRACTOR_FILE ${PROJECT_SOURCE_DIR}/libsrc/semiIndexExtractor/*.cpp)
message(STATUS "SEMIINDEXEXTRACTOR_SOURCES: " ${SEMIINDEXEXTRACTOR_FILE})
add_library(semiIndexExtractor STATIC ${SEMIINDEXEXTRACTOR_FILE})
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

file(GLOB OSTREE_FILE ${PROJECT_SOURCE_DIR}/libsrc/ostree/*.cpp)
message(STATUS "OSTREE_SOURCES: " ${OSTREE_FILE})
add_library(ostree STATIC  ${OSTREE_FILE})

file(GLOB MAINTAINER_FILE ${PROJECT_SOURCE_DIR}/libsrc/maintainer/*.cpp)
message(STATUS "MAINTAINER_SOURCES: " ${MAINTAINER_FILE})
add_library(maintainer STATIC  ${MAINTAINER_FILE})

file(GLOB SEMIINDEXEXTRACTOR_FILE ${PROJECT_SOURCE_DIR}/libsrc/semiIndexExtractor/*.cpp)
message(STATUS "SEMIINDEXEXTRACTOR_SOURCES: " ${SEMIINDEXEXTRACTOR_FILE})
add_library(semiIndexExtractor STATIC ${SEMIINDEXEXTRACTOR_FILE})
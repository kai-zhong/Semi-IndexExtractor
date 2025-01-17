aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

add_executable(main ${SRC_LIST})

target_link_libraries(main
${LIBRARY_OUTPUT_PATH}/libmaintainer.a
${LIBRARY_OUTPUT_PATH}/libutil.a
${LIBRARY_OUTPUT_PATH}/libgraph.a
${LIBRARY_OUTPUT_PATH}/libsemiIndexExtractor.a
${LIBRARY_OUTPUT_PATH}/libmbptree.a
${LIBRARY_OUTPUT_PATH}/libmaintainer.a
${LIBRARY_OUTPUT_PATH}/libostree.a
${OPENSSL_LIBRARIES}
)
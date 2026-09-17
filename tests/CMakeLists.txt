add_executable(hammer_smoke_test smoke_test.c)
target_link_libraries(hammer_smoke_test PRIVATE hammer_static hammer_warnings)
target_compile_definitions(hammer_smoke_test PRIVATE
    HAMMER_SMOKE_TEST_MODEL_PATH="${CMAKE_SOURCE_DIR}/external/cgltf/fuzz/data/Box.glb"
)

add_test(NAME hammer_smoke_test COMMAND hammer_smoke_test)

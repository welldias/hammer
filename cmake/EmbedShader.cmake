# Converts a compiled SPIR-V binary into a C header exposing it as a byte
# array, so hammer never needs to load shader files from disk at runtime.
#
# Required variables (passed via -D on the command line):
#   INPUT_FILE  - path to the .spv file to embed
#   OUTPUT_FILE - path to the .h file to generate
#   SYMBOL      - C identifier for the generated array

file(READ "${INPUT_FILE}" hex_content HEX)
string(LENGTH "${hex_content}" hex_length)
math(EXPR byte_count "${hex_length} / 2")

set(array_body "")
if(byte_count GREATER 0)
    math(EXPR last_byte "${byte_count} - 1")
    foreach(i RANGE 0 ${last_byte})
        math(EXPR hex_offset "${i} * 2")
        string(SUBSTRING "${hex_content}" ${hex_offset} 2 byte_hex)
        string(APPEND array_body "0x${byte_hex},")
    endforeach()
endif()

file(WRITE "${OUTPUT_FILE}"
"#pragma once\n"
"#include <stddef.h>\n"
"\n"
"alignas(4) static const unsigned char ${SYMBOL}[] = {${array_body}};\n"
"static const size_t ${SYMBOL}_size = sizeof(${SYMBOL});\n"
)

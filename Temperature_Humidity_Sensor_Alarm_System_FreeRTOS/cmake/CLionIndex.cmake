# Analysis-only declaration view derived from this project's RVDS header.
# This does not install, replace or implement a FreeRTOS port.
set(_rvds_portmacro
    "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/RVDS/ARM_CM3/portmacro.h")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_rvds_portmacro}")
file(READ "${_rvds_portmacro}" _rvds_header)

string(FIND "${_rvds_header}"
    "static portFORCE_INLINE void vPortSetBASEPRI" _inline_start)
string(REGEX MATCHALL
    "static portFORCE_INLINE [A-Za-z_][A-Za-z0-9_ \t]*\\([^)]*\\)"
    _inline_signatures "${_rvds_header}")
list(LENGTH _inline_signatures _inline_count)
if(_inline_start LESS 0 OR NOT _inline_count EQUAL 5)
    message(FATAL_ERROR
        "RVDS portmacro.h changed: review the CLion declaration view generator")
endif()

# Preserve real types, config branches and API macros preceding the assembly.
string(SUBSTRING "${_rvds_header}" 0 ${_inline_start} _index_header)
string(REPLACE "#define portFORCE_INLINE __forceinline"
    "#define portFORCE_INLINE inline" _index_header "${_index_header}")
string(PREPEND _index_header
    "/* Generated CLion declaration view; NOT a firmware port. */\n"
    "#ifndef CLION_CODE_INDEX\n"
    "#error This header is for CLion code indexing only\n"
    "#endif\n")
foreach(_signature IN LISTS _inline_signatures)
    string(REPLACE "static portFORCE_INLINE " "extern " _declaration "${_signature}")
    string(APPEND _index_header "${_declaration};\n")
endforeach()

# Keil intrinsic names used by preserved macros. Analysis needs declarations
# only; no no-op implementations or fake firmware are provided.
string(APPEND _index_header [=[

extern void __dsb(unsigned int barrier);
extern void __isb(unsigned int barrier);
extern uint32_t __clz(uint32_t value);

#ifdef __cplusplus
}
#endif
#endif /* PORTMACRO_H */
]=])

set(clion_index_include "${CMAKE_CURRENT_BINARY_DIR}/clion-index-include")
file(MAKE_DIRECTORY "${clion_index_include}")
file(CONFIGURE OUTPUT "${clion_index_include}/portmacro.h"
    CONTENT "${_index_header}" @ONLY)

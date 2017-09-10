file(STRINGS SOURCES src)
  

add_library (libwma ${src})

target_include_directories (libwma PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libwma INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libwma PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libwma codecs)

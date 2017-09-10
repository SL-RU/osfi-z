file(STRINGS SOURCES src)
  

add_library (libspc ${src})

target_include_directories (libspc PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cpu
  )
target_include_directories (libspc INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cpu
  )
set_target_properties(libspc PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libspc codecs)

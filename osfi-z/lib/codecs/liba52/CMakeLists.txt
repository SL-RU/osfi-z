file(STRINGS SOURCES src)
  

add_library (liba52 ${src})

target_include_directories (liba52 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (liba52 INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(liba52 PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(liba52 codecs)

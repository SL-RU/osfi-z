file(STRINGS SOURCES src)
  

add_library (libsetjmp ${src})

target_include_directories (libsetjmp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libsetjmp INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
TARGET_LINK_LIBRARIES(libsetjmp codecs)

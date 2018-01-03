file(STRINGS SOURCES src)
  

add_library (libpcm ${src})

target_include_directories (libpcm PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libpcm INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

TARGET_LINK_LIBRARIES(libpcm codecs stddeflib)

file(STRINGS SOURCES s)

set (h
  ${CMAKE_CURRENT_SOURCE_DIR} )

add_library(codec ${s})
target_include_directories(codec
  INTERFACE ${h})

target_include_directories(codec
  PUBLIC ${h})

target_link_libraries(codec hardware codecs metadata rockbox_sys fatfs stddeflib)



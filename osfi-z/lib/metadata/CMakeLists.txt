file(STRINGS SOURCES s)

set (h
  ${CMAKE_CURRENT_SOURCE_DIR} )

add_library(metadata ${s})
target_include_directories(metadata
  INTERFACE ${h})

target_include_directories(metadata
  PUBLIC ${h})

target_link_libraries(metadata codecs codec rockbox_sys fixedpoint stddeflib)

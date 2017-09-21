file(STRINGS SOURCES s)


set (metadata_h
  ${CMAKE_CURRENT_SOURCE_DIR} )

add_library(metadata ${s})
target_include_directories(metadata
  INTERFACE ${metadata_h})

target_include_directories(metadata
  PUBLIC ${metadata_h})

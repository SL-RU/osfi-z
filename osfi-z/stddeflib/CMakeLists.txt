file(STRINGS SOURCES s)

set (h
  ${CMAKE_CURRENT_SOURCE_DIR} )

add_library(stddeflib ${s})
target_include_directories(stddeflib
  INTERFACE ${h})

target_include_directories(stddeflib
  PUBLIC ${h})

target_link_libraries(stddeflib hardware fatfs rockbox_sys)

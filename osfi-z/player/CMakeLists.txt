file(STRINGS SOURCES s)

set (h
  ${CMAKE_CURRENT_SOURCE_DIR} )

add_library(player ${s})
target_include_directories(player
  INTERFACE ${h})

target_include_directories(player
  PUBLIC ${h})

target_link_libraries(player hardware codecs metadata rockbox_sys fatfs stddeflib gui MakiseGUI)



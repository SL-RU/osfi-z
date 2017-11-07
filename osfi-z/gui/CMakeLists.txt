file(STRINGS SOURCES s)

set (h
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/driver 
  ${CMAKE_CURRENT_SOURCE_DIR}/windows )

add_library(gui ${s})
target_include_directories(gui
  INTERFACE ${h})

target_include_directories(gui
  PUBLIC ${h})



TARGET_LINK_LIBRARIES(gui hardware MakiseGUI fatfs controls stddeflib player)
target_compile_definitions(gui PUBLIC ${GLOBAL_DEFS})

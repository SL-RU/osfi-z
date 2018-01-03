file(STRINGS SOURCES s)

set (h
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers )

add_library(fatfs ${s})
target_include_directories(fatfs
  INTERFACE ${h})

target_include_directories(fatfs
  PUBLIC ${h})


TARGET_LINK_LIBRARIES(fatfs hardware)

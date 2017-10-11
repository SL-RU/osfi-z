# BSD 2-Clause License
# 
# Copyright (c) 2017, Alexander Lutsai <s.lyra@ya.ru>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Requirements:
# cmake > 2.6
# arm-none-eabi-gdb
# python3


#LOAD CUBE MX CONFIGURATION
#USING PYTHON SCRIPT CubeMX2_cmake.py
set(cubemx_dir ${CMAKE_CURRENT_LIST_DIR})
message("CubeMX dir: " ${cubemx_dir})
string(REPLACE " " "" cubemx_dir ${cubemx_dir})
execute_process(COMMAND bash "-c"
  "python ${cubemx_dir}/CubeMX2_cmake.py ${cubemx_dir}"
  OUTPUT_VARIABLE cubemx_conf
  RESULT_VARIABLE cubemx_res
  ERROR_VARIABLE  cubemx_err)
if(${cubemx_res} EQUAL "0")
  message("CubeMX ok")
else()
  message(FATAL_ERROR ${cubemx_err})
endif()

foreach(i ${cubemx_conf})
  message(${i})
endforeach()

#Read ignore list
IF(EXISTS "${cubemx_dir}/CMakeIgnore.txt")
  message("Ignore file exists")
  FILE(READ "CMakeIgnore.txt" ignore_s)
  STRING(REGEX REPLACE ";" "\\\\;" ignore_s "${ignore_s}")
  STRING(REGEX REPLACE "\n" ";" ignore_s "${ignore_s}")
  set(ignore "")
  FOREACH(iss ${ignore_s})
    IF(NOT ${iss} EQUAL "")
      set(ignore ${ignore} "${iss}")
    ENDIF()
  ENDFOREACH()
ELSE()
  set(ignore "")
ENDIF()
#ignore subdirs
FOREACH(iss ${sub_dirs})
  IF(NOT ${iss} EQUAL "")
    set(ignore ${ignore} "${iss}")
    #add subdir
    add_subdirectory ("${iss}")
  ENDIF()
ENDFOREACH()
#message(${ignore})

list(GET cubemx_conf 0 STM32_PRJ_NAME     )
list(GET cubemx_conf 1 STM32_PRJ_CFLAGS   )
list(GET cubemx_conf 2 STM32_PRJ_DEFS     )
list(GET cubemx_conf 3 STM32_PRJ_MCU      )
list(GET cubemx_conf 4 STM32_PRJ_LD_SCRIPT)

message("CUBE project name: " ${STM32_PRJ_NAME})
message("CUBE MCU: "          ${STM32_PRJ_MCU})
message("CUBE CFLAGS: "       ${STM32_PRJ_CFLAGS})
message("CUBE DEFs: "         ${STM32_PRJ_DEFS})
message("CUBE LD script "     ${STM32_PRJ_LD_SCRIPT})

string(REPLACE " " "" STM32_PRJ_NAME ${STM32_PRJ_NAME})
string(REPLACE "\"" "" STM32_PRJ_DEFS ${STM32_PRJ_DEFS})
string(REPLACE " " ";" STM32_PRJ_DEFS ${STM32_PRJ_DEFS})
string(REPLACE " " "" STM32_PRJ_LD_SCRIPT ${STM32_PRJ_LD_SCRIPT})
string(REPLACE "\n" "" STM32_PRJ_LD_SCRIPT ${STM32_PRJ_LD_SCRIPT})
set(STM32_PRJ_LD_SCRIPT "${cubemx_dir}/${STM32_PRJ_LD_SCRIPT}")

#find and add all headers & sources & asm to target
MACRO(HEADER_DIRECTORIES return_list)
  FILE(GLOB_RECURSE new_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
  SET(dir_list "")
  FOREACH(file_path ${new_list})
    GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
    set(f 1)
    FOREACH(iss ${ignore})
      string(REGEX MATCH "^${iss}.*$" ouu ${dir_path})
      IF(NOT ${ouu} STREQUAL "")
	set(f 0)
      ENDIF()
    ENDFOREACH()
    IF(${f} EQUAL 1)
      #MESSAGE("Header +" ${dir_path})
      SET(dir_list ${dir_list} ${dir_path})
    ENDIF()
  ENDFOREACH()
  LIST(REMOVE_DUPLICATES dir_list)
  SET(${return_list} ${dir_list})
ENDMACRO()

MACRO(SRC_FILES_ALL return_list ext)
  FILE(GLOB_RECURSE new_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${ext}")
  SET(file_list "")
  FOREACH(file_path ${new_list})
    set(f 1)
    FOREACH(iss ${ignore})
      string(REGEX MATCH "^${iss}.*$" ouu ${file_path})
      #message("o ${ouu}")
      IF(NOT ${ouu} STREQUAL "")
	#message("${file_path} - ${iss}")
	set(f 0)
      ENDIF()
    ENDFOREACH()
    IF(${f} EQUAL 1)
      #MESSAGE("src +" ${file_path})
      SET(file_list ${file_list} ${file_path})
    ENDIF()
  ENDFOREACH()
  LIST(REMOVE_DUPLICATES file_list)
  SET(${return_list} ${file_list})
ENDMACRO()


#HEADERS
header_directories(INC)
include_directories(${INC})

#SOURCES
src_files_all(SRC "*.c")

#ASSEMBLER files
src_files_all(asm_SRC "*.s")
set_source_files_properties(${asm_SRC} "-x assembler-with-cpp")

add_library(hardware INTERFACE)
#list all files
message("INCLUDES:")
set(incs "")
foreach(f ${INC})
  set(incs "${incs} -I${f}")
  message(${f})
endforeach()
message("SOURCES: ")
foreach(f ${SRC})
  message(${f})
endforeach()
message("DEFINITIONS: ")
foreach(def ${STM32_PRJ_DEFS})
  message(${def})
  add_definitions("${def}")
endforeach()

target_include_directories(hardware
  INTERFACE ${INC})
#target_compile_definitions(hardware
#  INTERFACE   "${STM32_PRJ_DEFS}"
#  )

#setup flags
set(CMAKE_C_FLAGS "${STM32_PRJ_CFLAGS} -dM -std=c99 -g -gdwarf-2 ${USER_CFLAGS} ${OPT} -fdata-sections -ffunction-sections -Wa,-mimplicit-it=thumb" CACHE INTERNAL "" FORCE)
set(LDSCRIPT "-T${CMAKE_CURRENT_SOURCE_DIR}/../linker_stm32f405.ld -Wl,-Map=foo.map -specs=nosys.specs -Wa,-mimplicit-it=thumb -Wl,--gc-sections -lnosys")
set(LINKER_FLAGS "-lc -lm  ${USER_LINKER_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")


set(IS_ARM YES)
set(DEFFS
  -DTARGET_NAME="ZISHAN 1"
  -DTARGET_ID=1
  -DROCKBOX
  -DMEMORYSIZE=1
  -DTARGET_NAME=sdlapp
  -DCONFIG_CODEC=SWCODEC
  -DARM_ARCH=7
  -Dasm=__asm__
  -DCPU_ARM=1
)
set(GLOBAL_DEFS "${STM32_PRJ_DEFS};${DEFFS}" PARENT_SCOPE)


SET(TARGET ${CMAKE_PROJECT_NAME})

#setup targets
get_directory_property(C_DEFS COMPILE_DEFINITIONS)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#.elf
add_executable(${TARGET}.elf ${SRC} ${asm_SRC})

TARGET_LINK_LIBRARIES(${TARGET}.elf codec gui controls fatfs)

#print size
add_custom_command(TARGET ${TARGET}.elf POST_BUILD COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}.elf)
  set_target_properties(${TARGET}.elf
    PROPERTIES
    LINK_FLAGS ${LDSCRIPT}
    #COMPILE_FLAGS "${CMAKE_C_FLAGS_T}"
    )
#other
ADD_CUSTOM_TARGET(${TARGET}.hex DEPENDS ${TARGET}.elf COMMAND ${CMAKE_OBJCOPY} -Oihex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}.hex)
ADD_CUSTOM_TARGET(${TARGET}.bin DEPENDS ${TARGET}.elf COMMAND ${CMAKE_OBJCOPY} -Obinary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}.bin)

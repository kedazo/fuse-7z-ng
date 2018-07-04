CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(CheckIncludeFiles)


PROJECT(fuse-7z-ng)
set(CMAKE_USE_RELATIVE_PATHS TRUE)

FIND_PACKAGE(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --long
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE VERSION_FROM_GIT
    RESULT_VARIABLE GIT_RESULT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT GIT_RESULT EQUAL 0)
    message(WARNING "git returned error ${VERSION_FROM_GIT}")
    set(VERSION_FROM_GIT "v0.0.0")
  endif()
else(GIT_FOUND)
  set(VERSION_FROM_GIT "v0.0.0")
endif(GIT_FOUND)


string(REGEX MATCH "^v?[0-9]+(\\.[0-9]+)+" VERSION ${VERSION_FROM_GIT})
string(REGEX MATCHALL "[0-9]+" PARSED_VER ${VERSION})

list(LENGTH PARSED_VER PARSED_VER_LEN)
if(PARSED_VER_LEN GREATER 0)
  list(GET PARSED_VER 0 VERSION_MAJOR)
else()
  set(VERSION_MAJOR 0)
endif()
if(PARSED_VER_LEN GREATER 1)
  list(GET PARSED_VER 1 VERSION_MINOR)
else()
  set(VERSION_MINOR 0)
endif()
if(PARSED_VER_LEN GREATER 2)
  list(GET PARSED_VER 2 VERSION_PATCH)
else()
  set(VERSION_PATCH 0)
endif()
if(PARSED_VER_LEN GREATER 3)
  list(GET PARSED_VER 3 VERSION_TWEAK)
else()
  set(VERSION_TWEAK 0)
endif()

set(VERSION_BIN "${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH}")
message(STATUS "fuse-7z-ng version: ${VERSION}")
message(STATUS "fuse-7z-ng bin version: ${VERSION_BIN}")

set(PACKAGE_NAME "fuse-7z-ng" CACHE STRING "Program name")
set(PACKAGE_VERSION ${VERSION})
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%ct
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE COMPTIME
        RESULT_VARIABLE GIT_RESULT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT GIT_RESULT EQUAL 0)
        set(COMPTIME "0000000000")
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%D
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE VERSION_EXPORT
        RESULT_VARIABLE GIT_RESULT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT GIT_RESULT EQUAL 0)
        set(VERSION_EXPORT "HEAD -> master")
    endif()
else(GIT_FOUND)
    set(COMPTIME "0000000000")
    set(VERSION_EXPORT "HEAD -> master")
endif(GIT_FOUND)
set(fuse-7z-ng_VERSION "${VERSION_EXPORT} ${VERSION_FROM_GIT}")
message(STATUS "version tag: ${fuse-7z-ng_VERSION}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darvin")
    set(MACOS TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(FREEBSD TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
endif()

CHECK_INCLUDE_FILES("sys/types.h;sys/stat.h;unistd.h" NEEDED_HEADERS_PRESENT LANGUAGE CXX)


set(Source_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")

find_package (FUSE REQUIRED)
#add_definitions (-D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26)

set(CMake_Misc_Dir "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

file(GLOB_RECURSE SRCFILES "${Source_dir}/*.c" "${Source_dir}/*.cpp")

if(WINDOWS)
    #making resource file
    set(rc_script "${Source_dir}/fuse_7z_ng.rc")
    set(rc_script_proto "${CMake_Misc_Dir}/fuse_7z_ng.rc.in")
    set(license_file "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
    file(READ ${license_file} LICENSE_TEXT)
    string(REPLACE "\"" "\"\"" LICENSE_TEXT ${LICENSE_TEXT})
    string(REPLACE "\r\n" "\n" LICENSE_TEXT ${LICENSE_TEXT})
    string(REPLACE "\n" "\\r\\n" LICENSE_TEXT ${LICENSE_TEXT})
    message(STATUS "generating resources: ${rc_script_proto} -> ${rc_script}")
    configure_file(${rc_script_proto} ${rc_script})
    file(GLOB_RECURSE resource_files "${Source_dir}/*.rc" "${Source_dir}/*.ico")
endif(WINDOWS)


if(MSVC_IDE)
    file(GLOB_RECURSE HDRFILES "${Source_dir}/*.h")
endif()

# config.h
set(CONFIG_FILENAME "config.h")
set(CONFIG_FILE "${Source_dir}/${CONFIG_FILENAME}")
mark_as_advanced(CONFIG_FILE)
set(CONFIG_FILE_PROTO "${CMake_Misc_Dir}/${CONFIG_FILENAME}.in")

set(FUSE_USE_VERSION 28 CACHE INTEGER "Service name")
set(fuse_7z_ng_PACKAGE_NAME "${PACKAGE_NAME}" CACHE STRING "Service name")
set(fuse_7z_ng_STANDARD_BLOCK_SIZE 512 CACHE INTEGER "Block size")

message(STATUS "generating config: ${CONFIG_FILE_PROTO} -> ${CONFIG_FILE}")
configure_file(${CONFIG_FILE_PROTO} ${CONFIG_FILE})

if(WINDOWS)
    set(win_syslog_dir "${CMAKE_CURRENT_SOURCE_DIR}/syslog-win32")
    file(GLOB_RECURSE win_syslog_sources "${win_syslog_dir}/*.cpp" "${win_syslog_dir}/*.c")
endif(WINDOWS)

set(lib7zip_lib "${lib7zip_binDir}/lib7zip.a")
message(STATUS "lib7zip lib is ${lib7zip_lib}")
message(STATUS "lib7zip include dir is ${lib7zip_includeDir}")

add_executable(fuse_7z_ng "${SRCFILES}" "${HDRFILES}" "${win_syslog_sources}" "${resource_files}")
target_include_directories(fuse_7z_ng PUBLIC "${lib7zip_includeDir}" "${win_syslog_dir}" "${FUSE_INCLUDE_DIR}")

target_link_libraries(fuse_7z_ng ${lib7zip_lib} "${FUSE_LIBRARIES}")

if(WINDOWS)
    #for win_syslog
    target_link_libraries(fuse_7z_ng "wsock32" "ws2_32" "ssp")
endif(WINDOWS)


add_definitions("-D_FILE_OFFSET_BITS=64")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    #-fstack-protector-strong causes crash
    #-flto
    #-pie
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wformat -Werror=format-security")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -mmitigate-rop -fno-common -fstack-check")
    add_definitions("-D_FORTIFY_SOURCE=2")
    
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcet -fsanitize=cfi")
    endif()
    set_target_properties(fuse_7z_ng PROPERTIES LINK_FLAGS "-Wl,-O1,--sort-common,--as-needed,--dynamicbase,--nxcompat,--export-all-symbols,-flto")
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(fuse_7z_ng PROPERTIES LINK_FLAGS "-Wl,--image-base,0x140000000")
    endif()
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /sdl /GS /SafeSEH /NXCOMPAT /dynamicbase /guard:cf /HIGHENTROPYVA")
    set_target_properties(fuse_7z_ng PROPERTIES LINK_FLAGS "/guard:cf")
else()
    message(FATAL_ERROR "The compiler is not supported")
endif()

if(MSVC_IDE)
   source_group("res" FILES  ${resource_files})
endif()

if(MSVC)
    set_source_files_properties(${SRCFILES} PROPERTIES LANGUAGE CXX)
else(MSVC)
    #set_source_files_properties(${SRCFILES} PROPERTIES LANGUAGE CXX)
endif(MSVC)

#target_compile_features(fuse_7z_ng PUBLIC cxx_decltype_auto)

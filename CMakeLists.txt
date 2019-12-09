cmake_minimum_required(VERSION 2.8.11)
PROJECT(smapi C)
set(smapi_VERSION 2.5.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build shared libs" ON)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "My multi config types" FORCE)
  ADD_DEFINITIONS(-g -DDEBUG -D_DEBUG)
else(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "My multi config types" FORCE)
  ADD_DEFINITIONS(-DNDEBUG)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

if (MSVC)
  if (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-D_DLL -D_MAKE_DLL)
  else()
    set(CompilerFlags
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
    )
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
  endif()
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../out/lib")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../out/bin")
  ADD_DEFINITIONS(-DWIN32 -D_WINDOWS -D_CONSOLE -D_CRT_SECURE_NO_WARNINGS)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/smapi;${CMAKE_CURRENT_SOURCE_DIR}/../huskylib")
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    list( APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../out/lib/Debug" )
  else(CMAKE_BUILD_TYPE MATCHES Debug)
    list( APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../out/lib/Release" )
  endif(CMAKE_BUILD_TYPE MATCHES Debug)
else ()
  include_directories("smapi")
  ADD_DEFINITIONS(-DHAS_SIGNAL_H)
  ADD_DEFINITIONS(-Wall -DUNIX)
endif (MSVC)


find_library(husky_LIB NAMES husky PATHS "${CMAKE_FIND_ROOT_PATH}")

message("-------------------------")
message("System            : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
message("Compiler          : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message("Prefix            : ${CMAKE_INSTALL_PREFIX} (run \"cmake . -DCMAKE_INSTALL_PREFIX:PATH=/other\" for other prefix)")
message("libhusky          : ${husky_LIB}")
message("Build shared      : ${BUILD_SHARED_LIBS}")
message("Build type        : ${CMAKE_CONFIGURATION_TYPES}")
message("-------------------------")

SET(smapi_SOURCES src/api_jam.c
  src/api_sdm.c
  src/msgapi.c
  src/sq_area.c
  src/sq_hash.c
  src/sq_help.c
  src/sq_idx.c
  src/sq_kill.c
  src/sq_lock.c
  src/sq_misc.c
  src/sq_msg.c
  src/sq_read.c
  src/sq_uid.c
  src/sq_write.c
  src/structrw.c
)

SET(smapi_HEADERS smapi/api_brow.h
  smapi/apidebug.h
  smapi/api_jam.h
  smapi/api_jamp.h
  smapi/api_sdm.h
  smapi/api_sdmp.h
  smapi/api_sq.h
  smapi/api_sqp.h
  smapi/msgapi.h
  smapi/old_msg.h
)

ADD_LIBRARY(smapi ${smapi_SOURCES})
set_target_properties(smapi PROPERTIES VERSION ${smapi_VERSION})
set_target_properties(smapi PROPERTIES PUBLIC_HEADER "${smapi_HEADERS}")
target_link_libraries(smapi ${husky_LIB})
if (NOT MSVC)
  INSTALL(TARGETS smapi
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/smapi
  )
  set(CPACK_GENERATOR ${CPACK_GENERATOR} TGZ)
endif (NOT MSVC)

set (CPACK_PACKAGE_VERSION_MAJOR "2")
set (CPACK_PACKAGE_VERSION_MINOR "5")
file(READ cvsdate.h CPACK_PACKAGE_VERSION_PATCH LIMIT 10 OFFSET 17)
string(SUBSTRING ${CPACK_PACKAGE_VERSION_PATCH} 0 10 CPACK_PACKAGE_VERSION_PATCH)
string(REGEX REPLACE "-" "" CPACK_PACKAGE_VERSION_PATCH ${CPACK_PACKAGE_VERSION_PATCH}) 
INCLUDE(CPack)

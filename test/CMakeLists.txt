#----------------------------------------------------------
# targets
#----------------------------------------------------------

#file(GLOB_RECURSE TARGET_SRC *.cpp .)
file(GLOB TARGET_SRC *.cpp .)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(TARGET_NAME testnet1-dbg)
	set(LIBRARY o3dnet-dbg)
elseif (${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
	set(TARGET_NAME testnet1-odbg)
	set(LIBRARY o3dnet-odbg)
elseif (${CMAKE_BUILD_TYPE} MATCHES "Release")
	set(TARGET_NAME testnet1)
	set(LIBRARY o3dnet)
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_executable(${TARGET_NAME} ${TARGET_SRC})
target_link_libraries(${TARGET_NAME} ${LIBRARY} ${WSOCK32} ${OBJECTIVE3D_LIBRARY})

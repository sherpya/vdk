cmake_minimum_required(VERSION 3.6)
project(vdk)

set(WDK_WINVER "0x0A00")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(WDK REQUIRED)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vdkcmd)

include_directories(${CMAKE_SOURCE_DIR}/inc)


add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vdkmsg.h
	DEPENDS ${CMAKE_SOURCE_DIR}/win/cmd/vdkmsg.mc
	COMMAND mc
	ARGS
		-h ${CMAKE_CURRENT_BINARY_DIR}
		-r ${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_SOURCE_DIR}/win/cmd/vdkmsg.mc
	VERBATIM)

file(GLOB cmn_srcs ${CMAKE_SOURCE_DIR}/cmn/*.c ${CMAKE_SOURCE_DIR}/cmn/*.cpp)
file(GLOB vdkctl_srcs ${CMAKE_SOURCE_DIR}/win/lib/*.c)
set(vdkcmd_srcs
	${CMAKE_SOURCE_DIR}/win/cmd/vdkcmd.c
	${CMAKE_SOURCE_DIR}/win/cmd/vdkcmd.rc
	${CMAKE_CURRENT_BINARY_DIR}/vdkmsg.h)
message(${vdkcmd_srcs})
add_executable(vdkcmd ${vdkcmd_srcs} ${cmn_srcs} ${vdkctl_srcs})
target_include_directories(vdkcmd PRIVATE
	${CMAKE_SOURCE_DIR}/win/lib
	${CMAKE_SOURCE_DIR}/win/cmd
	${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(vdkcmd version)
set_target_properties(vdkcmd PROPERTIES OUTPUT_NAME vdk)

set(vdkdrv_srcs vdkopen.c vdkclose.c vdkread.c vdkwrite.c)
list(TRANSFORM vdkdrv_srcs PREPEND ${CMAKE_SOURCE_DIR}/cmn/)
wdk_add_library(vdkdrv STATIC WINVER 0x0A00 ${vdkdrv_srcs})
target_compile_definitions(vdkdrv PRIVATE VDK_KERNEL_DRIVER)

file(GLOB drv_srcs ${CMAKE_SOURCE_DIR}/drv/*.c ${CMAKE_SOURCE_DIR}/drv/vdkdrv.rc)
wdk_add_driver(vdk WINVER 0x0A00 ${drv_srcs})

target_compile_definitions(vdk PRIVATE VDK_KERNEL_DRIVER VDK_SUPPORT_NETWORK)
target_link_libraries(vdk vdkdrv)

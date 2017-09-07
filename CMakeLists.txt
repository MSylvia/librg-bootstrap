cmake_minimum_required(VERSION 2.8)
project(librg_bootstrap)

find_package(Threads)

# librg setup

# LIBRG_VENDOR_FOLDER can be changed to folder containing all dependencies
set(LIBRG_VENDOR_FOLDER "${CMAKE_SOURCE_DIR}/node_modules") 

# if LIBRG_VENDOR_FOLDER was changed, you also have to provide LIBRG_POSTFIX
set(LIBRG_POSTFIX ".c") 

add_subdirectory(node_modules/librg.c)


# adding executables
add_executable(server src/server.c)
add_executable(client src/client.c)

# link enet
target_link_libraries(server enet ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client enet ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

# maintain include directories
include_directories(node_modules/librg.c/include
	${LIBRG_VENDOR_FOLDER}/zpl${LIBRG_POSTFIX}/include
    ${LIBRG_VENDOR_FOLDER}/zpl_ent${LIBRG_POSTFIX}/include
    ${LIBRG_VENDOR_FOLDER}/zpl_math${LIBRG_POSTFIX}/include
    ${LIBRG_VENDOR_FOLDER}/zpl_cull${LIBRG_POSTFIX}/include
    ${LIBRG_VENDOR_FOLDER}/zpl_event${LIBRG_POSTFIX}/include
    ${LIBRG_VENDOR_FOLDER}/enet${LIBRG_POSTFIX}/include)

# on Windows, link against ws2_32 and winmm
if (WIN32)
    target_link_libraries(server ws2_32 winmm)
    target_link_libraries(client ws2_32 winmm)
endif()

# *nix systems need to link against math library
if (UNIX)
    target_link_libraries(server m)
    target_link_libraries(client m)
endif()
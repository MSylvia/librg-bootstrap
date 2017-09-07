cmake_minimum_required(VERSION 2.8)
project(librg_bootstrap)

# include it
add_subdirectory(node_modules/librg.c)

# adding out executables
add_executable(server src/server.c)
add_executable(client src/client.c)

# link needed libraries
target_link_libraries(server librg)
target_link_libraries(client librg)

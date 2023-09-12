cmake_minimum_required(VERSION 3.16.0)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXTRA_COMPONENT_DIRS "./modules" "./commands" "./src")
project(BCD-0o27_framework)
set(version 2.0.0)
spiffs_create_partition_image(spiffs data)

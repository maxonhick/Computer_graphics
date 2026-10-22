cmake_minimum_required(VERSION 3.20)
project(SFMLApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск SFML
find_package(SFML COMPONENTS Graphics Window System REQUIRED)

add_executable(sfml_app main.cpp)

# Линковка библиотек
target_link_libraries(sfml_app PRIVATE SFML::Graphics SFML::Window SFML::System)
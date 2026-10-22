cmake_minimum_required(VERSION 3.20)
project(SFMLApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS Graphics Window System REQUIRED)

find_package(OpenCV REQUIRED)

add_executable(sfml_app main.cpp)

target_include_directories(sfml_app PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(sfml_app PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System
    ${OpenCV_LIBS}
)
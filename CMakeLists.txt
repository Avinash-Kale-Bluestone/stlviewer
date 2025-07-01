cmake_minimum_required(VERSION 3.15)

project(STLViewer)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)

find_package(glad CONFIG REQUIRED)

find_package(imgui CONFIG REQUIRED)

find_package(assimp CONFIG REQUIRED)
    
find_package(OpenGL REQUIRED )
 
find_package(CGAL CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)

add_executable(viewer main.cpp)

target_link_libraries(viewer PRIVATE 
    CGAL::CGAL
    glfw          
    glad::glad
    imgui::imgui
    assimp::assimp
    OpenGL::GL
    glm::glm
)
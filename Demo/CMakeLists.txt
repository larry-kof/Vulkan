cmake_minimum_required (VERSION 3.8)
project(Demo)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")
find_package(Vulkan)
find_package(glfw3 REQUIRED)

include_directories( "../src/"
                    ${Vulkan_INCLUDE_DIR}
                    /usr/local/include  )
set (CMAKE_CXX_STANDARD 17)

# set(ALL_LIBS  ${Vulkan_LIBRARY} glfw VulkanTools)
set(ALL_LIBS  VulkanTools)

link_directories( "/usr/local/lib" )

add_executable( computeDemo compute_example.cpp )
add_executable( depthDemo depth_example.cpp )
add_executable( textureDemo texture.cpp )
add_executable( triangleDemo triangle.cpp )
add_executable( offscreenDemo offscreen_example.cpp )

target_link_libraries( computeDemo ${ALL_LIBS})
target_link_libraries( depthDemo ${ALL_LIBS})
target_link_libraries( textureDemo ${ALL_LIBS})
target_link_libraries( triangleDemo ${ALL_LIBS})
target_link_libraries( offscreenDemo ${ALL_LIBS})

add_custom_command( TARGET computeDemo
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    make_directory "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/shaders"
    COMMAND ${CMAKE_COMMAND} -E
    copy_directory
    "${PROJECT_SOURCE_DIR}/../shaders/"
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/shaders"
    COMMAND ${CMAKE_COMMAND} -E
    copy "${PROJECT_SOURCE_DIR}/../texture*.jpg"
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}"
    )

# add_dependencies( computeDemo vulkanTools )
# add_dependencies( depthDemo vulkanTools )
# add_dependencies( textureDemo vulkanTools )
# add_dependencies( triangleDemo vulkanTools )
cmake_minimum_required (VERSION 3.8)
project ( VulkanTools )

find_package(Vulkan)

# get rid of annoying MSVC warnings.

set (CMAKE_CXX_STANDARD 17)

include_directories(${Vulkan_INCLUDE_DIR} 
                    /usr/local/include )

set(ALL_LIBS  ${Vulkan_LIBRARY} glfw)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

file( GLOB SRC_FILES "*.cpp" "*.hpp" )

link_directories("/usr/local/lib")
# add_library( VulkanTools SHARED ${SRC_FILES})
add_library( VulkanTools ${SRC_FILES})

target_link_libraries(VulkanTools ${ALL_LIBS} )
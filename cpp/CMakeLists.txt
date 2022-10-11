cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
project(steam-shortcuts-editor VERSION 0.1.0)

#set (CMAKE_CXX_FLAGS "-lstdc++fs")

include(CTest)
enable_testing()
set(Boost_USE_STATIC_LIBS   ON)
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
#FIND_PACKAGE( Filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
#INCLUDE_DIRECTORIES( ${Filesystem_DIR} )

file(GLOB_RECURSE all_sources CONFIGURE_DEPENDS "src/*.hpp" "src/*.cpp")

add_executable(steam-shortcuts-editor ${all_sources})
add_compile_definitions("PROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")
add_compile_definitions("PROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")
add_compile_definitions("PROJECT_AUTHOR_NAME=\"${PROJECT_AUTHOR_NAME}\"")
add_compile_definitions("PROJECT_AUTHOR_EMAIL=\"${PROJECT_AUTHOR_EMAIL}\"")
add_compile_definitions("PROJECT_HOMEPAGE=\"${PROJECT_HOMEPAGE}\"")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(STATIC -static)
else()
    #add_compile_definitions("DEBUG=$<CONFIG:Debug>")
    add_compile_definitions("DEBUG=1")
endif()

TARGET_LINK_LIBRARIES( steam-shortcuts-editor LINK_PUBLIC ${Boost_LIBRARIES} stdc++fs ${STATIC})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


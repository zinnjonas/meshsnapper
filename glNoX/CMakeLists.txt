cmake_minimum_required(VERSION 3.3)

set( PROJECT_NAME meshsnapper)

project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp image.cpp mesh.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} X11)

find_package(ImageMagick COMPONENTS Magick++ montage MagickCore convert)

if( ImageMagick_FOUND)
    include_directories(${ImageMagick_Magick++_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${ImageMagick_montage_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${ImageMagick_LIBRARIES})
    add_definitions(-DIMAGE_MAGICK=1)
    add_definitions(-DPNG_LIB=0)
    add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16)
    add_definitions(-DMAGICKCORE_HDRI_ENABLE=1)
else()
    find_package(PNG REQUIRED)
    add_definitions(-DPNG_LIB=1)
    add_definitions(-DIMAGE_MAGICK=0)
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARY})
endif()


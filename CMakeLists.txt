cmake_minimum_required(VERSION 3.3)
project(meshsnapper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp image.h image.cpp mesh.cpp mesh.h)
add_executable(meshsnapper ${SOURCE_FILES})

find_package(GLUT REQUIRED)

include_directories(${GLUT_INCLUDE_DIR})
target_link_libraries(meshsnapper ${GLUT_LIBRARY})

find_package(GLU REQUIRED)

include_directories(${GLU_INCLUDE_DIR})
target_link_libraries(meshsnapper ${GLU_LIBRARY})

find_package(OpenGL REQUIRED)

include_directories(${OpenGL_INCLUDE_DIR})
target_link_libraries(meshsnapper ${OpenGL_LIBRARY})

find_package(ImageMagick COMPONENTS Magick++ montage MagickCore convert)

if( ImageMagick_FOUND)
    include_directories(${ImageMagick_Magick++_INCLUDE_DIR})
    target_link_libraries(meshsnapper ${ImageMagick_montage_LIBRARIES})
    target_link_libraries(meshsnapper ${ImageMagick_LIBRARIES})
    add_definitions(-DIMAGE_MAGICK=1)
    add_definitions(-DPNG_LIB=0)
    add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16)
    add_definitions(-DMAGICKCORE_HDRI_ENABLE=1)
else()
    find_package(PNG REQUIRED)
    add_definitions(-DPNG_LIB=1)
    add_definitions(-DIMAGE_MAGICK=0)
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries(meshsnapper ${PNG_LIBRARY})
endif()

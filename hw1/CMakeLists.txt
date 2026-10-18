cmake_minimum_required(VERSION 3.10)
project(hw1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Eigen3 3 REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(hw1 main.cpp rasterizer.cpp)
target_link_libraries(hw1 PRIVATE Eigen3::Eigen ${OpenCV_LIBS})

# 自检目标:内含参考实现,用来"对答案"。
# 默认不编译,做完作业想验证时用:
#   cmake -S . -B build -DHW1_SELFCHECK=ON && cmake --build build -j8 && ./build/selfcheck
option(HW1_SELFCHECK "编译自检目标(内含参考矩阵,建议先做完作业再看)" OFF)
if(HW1_SELFCHECK)
  add_executable(selfcheck tests/reference_check.cpp rasterizer.cpp)
  target_include_directories(selfcheck PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(selfcheck PRIVATE Eigen3::Eigen ${OpenCV_LIBS})
endif()

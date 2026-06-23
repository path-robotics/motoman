cmake_minimum_required(VERSION 3.12)

project(motoman_sia20d_moveit_config NONE)

find_package(ament_cmake REQUIRED)
ament_package()

install(DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME})

cmake_minimum_required(VERSION 3.12)

project(motoman_ma2010_moveit_config NONE)

find_package(ament_cmake REQUIRED)
ament_package()

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME}
  PATTERN "setup_assistant.launch" EXCLUDE)
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})

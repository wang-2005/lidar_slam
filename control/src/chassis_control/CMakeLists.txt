cmake_minimum_required(VERSION 3.8)
project(chassis_control)

# 1. 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(serial REQUIRED)

# 2. 包含头文件路径
include_directories(
  include
  ${rclcpp_INCLUDE_DIRS}
  ${geometry_msgs_INCLUDE_DIRS}
  ${serial_INCLUDE_DIRS}
)

# 3. 生成可执行文件
add_executable(serial_bridge_node
  src/control_bridge.cpp
  src/protocol_chassis.c
)

# 4. 链接库 (改用这种更直接的写法)
target_link_libraries(serial_bridge_node
  ${rclcpp_LIBRARIES}
  ${geometry_msgs_LIBRARIES}
  ${serial_LIBRARIES}
)

# 5. 确保依赖包先编译
ament_target_dependencies(serial_bridge_node
  rclcpp
  geometry_msgs
  serial
)

# 6. 安装路径修正
install(TARGETS
  serial_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
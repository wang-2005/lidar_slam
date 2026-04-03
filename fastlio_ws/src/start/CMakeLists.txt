cmake_minimum_required(VERSION 3.8)
project(start)

find_package(ament_cmake REQUIRED)

# 关键：将当前目录下的所有 .py 文件安装到该包的路径下
install(FILES
  qidong.launch.py
  DESTINATION share/${PROJECT_NAME}
)

ament_package()

import os
from datetime import datetime
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # 1. 定义地图保存目录 (改为你现在的 ICP 项目路径)
    pcd_save_dir = "/home/wang/MID360_Project/fastlio_ICP/PCD"
    if not os.path.exists(pcd_save_dir):
        os.makedirs(pcd_save_dir)

    # 2. 生成唯一文件名
    now = datetime.now().strftime("%Y%m%d_%H%M%S")
    unique_map_path = os.path.join(pcd_save_dir, f"map_{now}.pcd")

    # 3. 获取配置路径
    # 注意：这里 FindPackageShare 找的是你现在的包名 fastlio2
    package_path = get_package_share_directory('fastlio2')
    rviz_cfg = os.path.join(package_path, 'rviz', 'fastlio2.rviz')
    config_path = os.path.join(package_path, 'config', 'mid360.yaml')

    lio_node = Node(
        package="fastlio2",
        namespace="fastlio2", # 必须有，因为代码里的参数解析可能带了这个前缀
        executable="lio_node",
        name="lio_node",
        output="screen",
        parameters=[{"config_path": config_path}] 
    )


    # 5. 配置 RViz 节点
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", rviz_cfg]
    )

    return LaunchDescription([
        lio_node,
        rviz_node
    ])
import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # 获取功能包路径
    # 注意：确保这些包已经安装或在你的工作空间中
    livox_driver_dir = get_package_share_directory('livox_ros_driver2')
    fast_lio_dir = get_package_share_directory('fast_lio')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')

    # 1. 启动 Mid360 雷达驱动
    livox_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(livox_driver_dir, 'launch_ROS2', 'msg_MID360_launch.py')
        )
    )

    # 2. 启动 FAST-LIO 定位算法
    # 这里直接指向你已经改好 timestamp_unit: 1 的 mid360.yaml
    fast_lio_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(fast_lio_dir, 'launch', 'map.launch.py')
        ),
        launch_arguments={
            # 'config_file': '/home/wang/MID360_Project/fastlio_ws/src/FAST_LIO/config/mid360.yaml'
            'config_file': 'mid360.yaml',
            'rviz': 'false'
        }.items()
    )

    # 3. 启动点云转激光 (用于 Nav2 避障和 AMCL 重定位)
    pcl_to_scan_node = Node(
        package='pointcloud_to_laserscan',
        executable='pointcloud_to_laserscan_node',
        name='pointcloud_to_laserscan',
        parameters=[{
            'min_height': -0.7,
            'max_height': 0.5,
            'angle_min': -3.14159,
            'angle_max': 3.14159,
            'range_max': 30.0,
            'use_sim_time': False
        }],
        remappings=[('cloud_in', '/cloud_registered')]
    )

    # 4. 启动 Nav2 定位模块 (AMCL + Map Server)
    nav2_localization = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(nav2_bringup_dir, 'launch', 'localization_launch.py')
        ),
        launch_arguments={
            'map': '/home/wang/MID360_Project/maps/map5.yaml',
            'use_sim_time': 'false',
            'params_file': '/home/wang/MID360_Project/nav2_params.yaml',
            # 新增：设置初始位姿，默认原点
            'initial_pose_x': '0.0',
            'initial_pose_y': '0.0',
            'initial_pose_z': '0.0',
            'initial_pose_yaw': '0.0'
        }.items()
    )

    return LaunchDescription([
        livox_launch,
        fast_lio_launch,
        pcl_to_scan_node,
        nav2_localization
    ])

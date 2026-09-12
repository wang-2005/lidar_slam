"""Mid-360 -> FAST-LIO2 -> ICP loop closure / pose-graph optimization.

Only PGO owns map -> lidar in this mode. Do not also launch localizer.
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    lio_config = LaunchConfiguration("lio_config")
    pgo_config = LaunchConfiguration("pgo_config")
    start_driver = LaunchConfiguration("start_driver")
    start_rviz = LaunchConfiguration("start_rviz")

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "lio_config",
                default_value=PathJoinSubstitution(
                    [FindPackageShare("fastlio2"), "config", "mid360.yaml"]
                ),
                description="FAST-LIO2 config with Mid-360 topics and LiDAR/IMU extrinsics",
            ),
            DeclareLaunchArgument(
                "pgo_config",
                default_value=PathJoinSubstitution(
                    [FindPackageShare("pgo"), "config", "pgo.yaml"]
                ),
                description="ICP loop and GTSAM pose-graph parameters",
            ),
            DeclareLaunchArgument("start_driver", default_value="true"),
            DeclareLaunchArgument("start_rviz", default_value="false"),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    PathJoinSubstitution(
                        [
                            FindPackageShare("livox_ros_driver2"),
                            "launch_ROS2",
                            "msg_MID360_launch.py",
                        ]
                    )
                ),
                condition=IfCondition(start_driver),
            ),
            Node(
                package="fastlio2",
                executable="lio_node",
                namespace="fastlio2",
                output="screen",
                parameters=[{"config_path": lio_config}],
            ),
            Node(
                package="pgo",
                executable="pgo_node",
                namespace="pgo",
                output="screen",
                parameters=[{"config_path": pgo_config}],
            ),
            Node(
                package="rviz2",
                executable="rviz2",
                output="screen",
                arguments=[
                    "-d",
                    PathJoinSubstitution([FindPackageShare("pgo"), "rviz", "pgo.rviz"]),
                ],
                condition=IfCondition(start_rviz),
            ),
        ]
    )

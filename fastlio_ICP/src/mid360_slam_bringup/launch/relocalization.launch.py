"""Mid-360 -> FAST-LIO2 -> two-stage ICP map relocalization.

Only localizer owns map -> lidar in this mode. Do not also launch PGO.
Load a PCD map through /localizer/relocalize after startup.
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
    localizer_config = LaunchConfiguration("localizer_config")
    start_driver = LaunchConfiguration("start_driver")
    start_rviz = LaunchConfiguration("start_rviz")

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "lio_config",
                default_value=PathJoinSubstitution(
                    [FindPackageShare("fastlio2"), "config", "mid360.yaml"]
                ),
            ),
            DeclareLaunchArgument(
                "localizer_config",
                default_value=PathJoinSubstitution(
                    [FindPackageShare("localizer"), "config", "localizer.yaml"]
                ),
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
                package="localizer",
                executable="localizer_node",
                namespace="localizer",
                output="screen",
                parameters=[{"config_path": localizer_config}],
            ),
            Node(
                package="rviz2",
                executable="rviz2",
                output="screen",
                arguments=[
                    "-d",
                    PathJoinSubstitution(
                        [FindPackageShare("localizer"), "rviz", "localizer.rviz"]
                    ),
                ],
                condition=IfCondition(start_rviz),
            ),
        ]
    )

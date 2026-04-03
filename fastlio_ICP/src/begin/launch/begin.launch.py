import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction, ExecuteProcess
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node

def generate_launch_description():
    # 1. 获取各个包的路径
    localizer_share_dir = get_package_share_directory('localizer')

    # 2. 修改点：不仅要定义路径，还要定义“启动动作”
    livox_launch_path = '/home/wang/MID360_Project/livox_ws/install/livox_ros_driver2/share/livox_ros_driver2/launch_ROS2/msg_MID360_launch.py'
    # 新增这一步：将路径转换为可以运行的动作对象
    livox_launch_action = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(livox_launch_path)
    )

    # 3. 包含定位与算法的 launch 文件
    localizer_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(localizer_share_dir, 'launch', 'localizer_launch.py')
        )
    )

    # --- Pose Monitor 节点 ---
    pose_node = Node(
        package='pose',   # 功能包名        
        executable='pose_node',  # 对应 CMakeLists 里的 add_executable 名字
        name='pose_monitor',  # 覆盖（Override）代码里的节点名
        output='screen'           
    )

    # 4. 定义自动调用重定位服务的命令
    call_relocalize_service = ExecuteProcess(
        cmd=[
            'ros2', 'service', 'call', 
            '/localizer/relocalize', 
            'interface/srv/Relocalize', 
            '"{pcd_path: \'/home/wang/MID360_Project/fastlio_ICP/PCD/map2/map.pcd\'}"'
        ],
        shell=True,
        output='screen'
    )

    # 5. 设置延迟调用
    delayed_service_call = TimerAction(
        period=5.0,
        actions=[call_relocalize_service]
    )

    # 6. 修改点：确保列表里的变量名与上面定义的一致
    return LaunchDescription([
        livox_launch_action,    # 这里改成了上面定义的 action 变量名
        localizer_launch,       
        pose_node,
        delayed_service_call    
    ])

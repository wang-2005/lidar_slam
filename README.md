# Livox Mid-360 + FAST-LIO2：ICP 回环建图与地图重定位

本仓库集成 ROS 2 下的 FAST-LIO2、两阶段 ICP 地图重定位和基于 ICP 回环约束的 GTSAM/iSAM2 位姿图优化（PGO）。**建图和重定位是两个独立运行模式**：两者都会发布 `map -> lidar`，不可同时启动。这里的 ICP/PGO 核心算法来自下文注明的上游项目；本仓库的工作重点是 Mid-360 参数、工作空间和启动链路集成。

## 数据流与坐标系

| 模式 | 数据流 | `map -> lidar` 发布者 |
| --- | --- | --- |
| 回环建图 | Mid-360 `/livox/lidar` + `/livox/imu` → FAST-LIO2 `/fastlio2/body_cloud` + `/fastlio2/lio_odom` → ICP 回环检测 → GTSAM/iSAM2 位姿图优化 → PCD 地图 | `pgo_node` |
| 地图重定位 | Mid-360 → FAST-LIO2 → 对已存 PCD 地图做粗/精两阶段 ICP → 位置校正 | `localizer_node` |

FAST-LIO2 发布局部 `lidar -> body`；PGO 或重定位节点发布全局 `map -> lidar`。因此可通过 TF 查询 `map -> body`。PGO 的 ICP 用来确认回环并构造位姿图约束；重定位的 ICP 用来将当前点云对齐已知地图，二者不是同一个任务。

## 环境与构建

目标环境为 Ubuntu 22.04 + ROS 2 Humble。需要 PCL、Eigen3、Sophus、GTSAM、yaml-cpp、`message_filters`、`pcl_conversions`，以及 [Livox-SDK2](https://github.com/Livox-SDK/Livox-SDK2) 和 [livox_ros_driver2](https://github.com/Livox-SDK/livox_ros_driver2)。先按 Livox 官方说明安装 SDK2；驱动子模块固定在 `livox_ws/src/livox_ros_driver2`。本仓库还含其他实验性工作空间，本流程只构建下面列出的五个 ROS 包。

```bash
git clone --recurse-submodules https://github.com/wang-2005/lidar_slam.git
cd lidar_slam

# 先安装/构建 Livox-SDK2，然后按 Livox 驱动 README 构建 Humble 驱动。
cd livox_ws/src/livox_ros_driver2
source /opt/ros/humble/setup.bash
./build.sh humble
cd ../../..

source /opt/ros/humble/setup.bash
source livox_ws/install/setup.bash
cd fastlio_ICP
colcon build --symlink-install --packages-select interface fastlio2 pgo localizer mid360_slam_bringup
source install/setup.bash
```

如已克隆但未初始化子模块，在仓库根目录运行 `git submodule update --init --recursive`。构建前确认 CMake 能找到 Sophus/GTSAM；不要依赖仓库中历史提交的 `eigen_install` 二进制目录作为通用安装步骤。按实际网卡和雷达 IP 修改 Livox 驱动配置，并核对 `fastlio_ICP/src/FASTLIO2_ROS2/fastlio2/config/mid360.yaml` 的话题、时间戳及雷达—IMU 外参。

## A. 回环建图

在已 source 驱动和 `fastlio_ICP/install/setup.bash` 的终端执行：

```bash
ros2 launch mid360_slam_bringup mapping.launch.py start_driver:=true start_rviz:=true
```

如果 Livox 驱动已经在另一个终端运行，改为 `start_driver:=false`。默认使用 Mid-360 的 LIO 参数和 `pgo/config/pgo.yaml`，也可通过 `lio_config:=/绝对路径/配置.yaml`、`pgo_config:=/绝对路径/配置.yaml` 覆盖。`pgo_launch.py` 的旧入口同样已改用 `mid360.yaml`，但推荐使用上面的统一入口。

沿有重访的路径采集数据；只有形成符合搜索半径、时间间隔和 ICP 评分阈值的回环时，位姿图中才会添加回环约束。检查话题和 TF：

```bash
ros2 topic hz /fastlio2/lio_odom
ros2 topic echo /fastlio2/body_cloud --once
ros2 run tf2_ros tf2_echo lidar body
ros2 run tf2_ros tf2_echo map lidar
ros2 topic echo /pgo/loop_markers --once
```

`/pgo/loop_markers` 仅在检测到回环后才有标记。保存到一个**新的**输出目录（服务会创建目录，拒绝覆盖已有地图）：

```bash
ros2 service call /pgo/save_maps interface/srv/SaveMaps \
  "{file_path: '/data/mid360/run_001', save_patches: true}"
```

成功后应看到 `map.pcd`、`poses.txt` 和 `patches/`。路径 `/data/mid360/run_001` 只是示例，请换成当前用户可写的绝对路径。

## B. 对已有地图做 ICP 重定位

先停止建图模式，避免 PGO 和 localizer 同时发布 `map -> lidar`，然后启动：

```bash
ros2 launch mid360_slam_bringup relocalization.launch.py start_driver:=true start_rviz:=true
ros2 service call /localizer/relocalize interface/srv/Relocalize \
  "{pcd_path: '/data/mid360/run_001/map.pcd', x: 0.0, y: 0.0, z: 0.0, yaw: 0.0, pitch: 0.0, roll: 0.0}"
ros2 service call /localizer/relocalize_check interface/srv/IsValid "{code: 0}"
ros2 run tf2_ros tf2_echo map body
```

`/localizer/relocalize` 返回成功仅表示地图已加载、对齐请求已接受；以 `relocalize_check` 的 `valid: true` 及 TF 输出确认 ICP 实际收敛。初值 `x/y/z/yaw/pitch/roll` 应接近机器人在地图中的真实初始位姿，均以米/弧度表示；局部 ICP 不保证从任意远的初值完成全局搜索。

## 验证边界

本分支的静态检查覆盖启动拓扑、配置话题/坐标系和 Python 语法。**尚未在本机完成 ROS 2 编译、Mid-360 实机运行或导航闭环测试**；因此不声明厘米级定位精度、回环优化幅度或 Nav2 自主导航成功。实机验收建议留存 rosbag、回环前后轨迹/地图、重定位成功率与耗时、TF 树和终端日志。旧的 `fastlio_ws/src/start/qidong.launch.py` 是 FAST-LIO + AMCL/Nav2 定位实验入口，不属于上述 ICP/PGO 一体建图链路。

## 上游与许可

- [robotics-laboratory/fast-lio2](https://github.com/robotics-laboratory/fast-lio2)：本仓库 `fastlio_ICP/src/FASTLIO2_ROS2` 中 FAST-LIO2、ICP localizer 和 PGO 的主要来源。保留其各子包原有 LICENSE；本分支在其基础上增加 Mid-360 启动集成和必要的接口修正。
- [HKU-MARS FAST-LIO2](https://github.com/hku-mars/FAST_LIO)、[Livox ROS Driver 2](https://github.com/Livox-SDK/livox_ros_driver2) 等通过 Git 子模块引用，见 `.gitmodules`。各第三方组件遵循各自许可证。

仓库内历史地图和 `eigen_install` 属于旧实验产物，不能作为算法原创性或精度证明。本仓库自有集成代码的许可与第三方代码的许可应分别核对；不要将整库简单视作单一 MIT 项目。

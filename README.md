# MID360 + FAST-LIO2 SLAM 系统

![GitHub stars](https://img.shields.io/github/stars/yourusername/MID360-FAST-LIO2?style=social)
![GitHub forks](https://img.shields.io/github/forks/yourusername/MID360-FAST-LIO2?style=social)
![GitHub license](https://img.shields.io/github/license/yourusername/MID360-FAST-LIO2)

## 📑 项目简介

本项目是基于 **MID360激光雷达** 和 **FAST-LIO2** 算法的高性能 LiDAR-Inertial SLAM 系统，用于实现高精度的实时定位与建图。该系统通过紧密耦合的迭代扩展卡尔曼滤波器融合LiDAR特征点和IMU数据，在快速运动、嘈杂或杂乱环境中依然能够保持稳定的导航性能。

### 🚀 核心功能

- **实时定位与建图**：基于FAST-LIO2算法，实现高精度的实时定位和地图构建
- **多传感器融合**：紧密耦合LiDAR和IMU数据，提高系统鲁棒性
- **高效点云处理**：使用ikd-Tree实现快速点云搜索，支持高频率激光雷达数据处理
- **自动地图保存**：支持自动生成带时间戳的点云地图文件
- **重定位功能**：基于ICP算法实现机器人在已知环境中的精确定位
- **多种激光雷达支持**：兼容Livox MID360等多种激光雷达

## 📁 项目结构

```
MID360_Project/
├── Livox-SDK2/           # Livox激光雷达官方SDK
├── Localization/         # 定位功能实现
├── Sophus/               # 李代数库
├── control/              # 机器人控制系统
├── eigen/                # 线性代数库
├── eigen_install/        # Eigen库安装文件
├── fastlio_ICP/          # ICP算法实现（重定位功能）
│   ├── PCD/              # 重定位用的参考地图
│   └── 重定位              # 重定位相关代码
├── fastlio_ws/           # FAST-LIO2主要工作空间
│   ├── PCD/              # 点云地图文件
│   └── src/FAST_LIO/     # FAST-LIO2核心代码
├── gtsam/                # 因子图优化库
├── livox_ws/             # Livox ROS驱动工作空间
├── maps/                 # 栅格地图文件
└── pcd2pgm/              # 点云转栅格地图工具
```

### 📌 主要文件夹说明

| 文件夹 | 用途 | 重要性 |
|-------|------|--------|
| fastlio_ws/ | FAST-LIO2核心工作空间，包含算法实现和配置 | ⭐⭐⭐⭐⭐ |
| fastlio_ICP/ | 实现ICP算法，提供重定位功能 | ⭐⭐⭐⭐⭐ |
| livox_ws/ | Livox激光雷达ROS驱动，提供数据接口 | ⭐⭐⭐⭐ |
| PCD/ | 存储生成的点云地图文件 | ⭐⭐⭐ |
| maps/ | 存储转换后的栅格地图文件 | ⭐⭐ |
| Livox-SDK2/ | 激光雷达底层SDK | ⭐⭐⭐ |

## 🔧 环境要求

- **操作系统**：Ubuntu 20.04 或更高版本
- **ROS**：ROS 2 Foxy 或更高版本（推荐 ROS 2 Humble）
- **依赖库**：
  - PCL >= 1.8
  - Eigen >= 3.3.4
  - livox_ros_driver2

## 🛠️ 安装步骤

### 1. 克隆项目

```bash
git clone https://github.com/yourusername/MID360-FAST-LIO2.git
cd MID360-FAST-LIO2
```

### 2. 安装系统依赖

```bash
# 安装PCL和Eigen
sudo apt install libpcl-dev libeigen3-dev

# 安装ROS 2（以Humble为例）
# 参考：https://docs.ros.org/en/humble/Installation.html
```

### 3. 构建Livox-SDK2

```bash
cd Livox-SDK2
mkdir -p build && cd build
cmake .. && make -j4 && sudo make install
cd ../..
```

### 4. 构建livox_ws

```bash
cd livox_ws
colcon build --symlink-install
source install/setup.bash
cd ..
```

### 5. 构建fastlio_ws

```bash
cd fastlio_ws
colcon build --symlink-install
source install/setup.bash
cd ..
```

## 🚗 使用方法

### 1. 启动MID360激光雷达

```bash
cd livox_ws
source install/setup.bash
ros2 launch livox_ros_driver2 msg_MID360_launch.py
```

### 2. 启动FAST-LIO2 SLAM系统

```bash
cd fastlio_ws
source install/setup.bash
ros2 launch fast_lio mapping.launch.py config_file:=mid360.yaml
```

### 3. 地图操作

#### 3.1 保存地图

系统会自动在 `fastlio_ws/PCD/` 目录下生成带时间戳的点云地图文件（如 `map_20260320_171005.pcd`）。

**手动保存地图**：
```bash
ros2 service call /map_save std_srvs/srv/Trigger {}
```

#### 3.2 查看PCD地图

```bash
pcl_viewer test.pcd -ax 1
```

#### 3.3 PCD地图转PNG

1. **启动转换工具**：
   ```bash
   cd pcd2pgm
   ros2 launch pcd2pgm pcd2pgm.launch.py
   ```

2. **在RViz中查看**：
   ```bash
   rviz2
   ```

3. **保存PNG地图**：
   ```bash
   cd maps
   ros2 run nav2_map_server map_saver_cli -f map2
   ```

### 4. 重定位功能

重定位功能用于在已知环境中快速确定机器人的初始位置，详细步骤如下：

#### 4.1 重定位启动教程

1. **启动MID360激光雷达**：
   ```bash
   cd livox_ws
   ros2 launch livox_ros_driver2 msg_MID360_launch.py
   ```

2. **启动重定位算法**：
   ```bash
   cd fastlio_ICP
   source install/setup.bash
   ros2 launch localizer localizer_launch.py
   ```

3. **触发重定位**（在新终端中）：
   ```bash
   cd fastlio_ICP
   source install/setup.bash
   ros2 service call /localizer/relocalize interface/srv/Relocalize "{pcd_path: '/home/wang/MID360_Project/fastlio_ws/PCD/map4.pcd'}"
   ```

4. **查看重定位结果**：
   ```bash
   ros2 run tf2_ros tf2_echo map lidar
   ```

#### 4.2 回环建图方法

1. **启动MID360激光雷达**：
   ```bash
   cd livox_ws
   ros2 launch livox_ros_driver2 msg_MID360_launch.py
   ```

2. **启动回环建图**：
   ```bash
   cd fastlio_ICP
   ros2 launch pgo pgo_launch.py
   ```

3. **保存地图**：
   ```bash
   cd fastlio_ICP
   ros2 service call /pgo/save_maps interface/srv/SaveMaps "{file_path: '/home/wang/MID360_Project/fastlio_ICP/PCD/map2', save_patches: true}"
   ```

#### 4.3 一键启动

**一键启动（雷达硬件、重定位算法、实时位姿打印）**：

```bash
# 注意：需要在launch文件中更改地图路径
# 运行MID360并打开rviz2
cd livox_ws
ros2 launch livox_ros_driver2 rviz_MID360_launch.py

# 一键启动所有组件
cd fastlio_ws
ros2 launch start qidong.launch.py
```

**网络设置**：
- 地址：192.168.1.50
- 子网掩码：255.255.255.0
- 网关：192.168.1.1

## ⚙️ 配置说明

### 主要配置文件

| 配置文件 | 用途 | 路径 |
|---------|------|------|
| mid360.yaml | FAST-LIO2核心配置 | `fastlio_ws/src/FAST_LIO/config/mid360.yaml` |
| nav2_params.yaml | Nav2导航参数 | `nav2_params.yaml` |
| pcd.yaml | PCD转PGM配置 | `pcd2pgm/src/pcd2pgm/config/pcd.yaml` |
| MID360_config.json | MID360激光雷达配置 | `livox_ws/src/livox_ros_driver2/config/MID360_config.json` |

### 关键参数说明

#### mid360.yaml 关键参数：

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `lid_topic` | 激光雷达点云话题 | `/livox/lidar` |
| `imu_topic` | IMU数据话题 | `/livox/imu` |
| `extrinsic_T` | IMU到激光雷达的平移外参 | `[-0.011, -0.02329, 0.04412]` |
| `extrinsic_R` | IMU到激光雷达的旋转外参 | `[1., 0., 0., 0., 1., 0., 0., 0., 1.]` |
| `pcd_save.pcd_save_en` | 是否启用PCD文件保存 | `true` |
| `pcd_save.map_save_path` | 地图保存路径 | `/home/wang/MID360_Project/fastlio_ws/map.pcd` |

#### MID360_config.json 关键参数：

| 参数 | 说明 | 推荐值 |
|------|------|--------|
| `pcl_data_type` | 点云数据类型 | 1 (CustomMsg格式，用于FAST-LIO2) |

## 📊 技术特点

1. **计算高效**：使用ikd-Tree实现快速点云搜索，支持超过100Hz的激光雷达数据处理
2. **精度高**：直接使用原始LiDAR点云进行scan-to-map匹配，无需特征提取
3. **鲁棒性强**：紧密耦合的LiDAR-Inertial融合，在复杂环境中表现稳定
4. **通用性好**：支持多种激光雷达类型，包括旋转式和固态式
5. **易于部署**：支持ARM平台，如Khadas VIM3、Nvidia TX2、Raspberry Pi 4B

## 🎯 应用场景

- **机器人导航**：为移动机器人提供高精度定位和地图
- **自动驾驶**：为自动驾驶车辆提供环境感知和定位能力
- **无人机应用**：为无人机提供导航和避障能力
- **AR/VR**：为增强现实和虚拟现实提供空间定位

## 📚 参考资料

- [FAST-LIO2: Fast Direct LiDAR-inertial Odometry](https://github.com/hku-mars/FAST_LIO)
- [Livox-SDK2](https://github.com/Livox-SDK/Livox-SDK2)
- [livox_ros_driver2](https://github.com/Livox-SDK/livox_ros_driver2)
- [ikd-Tree](https://github.com/hku-mars/ikd-Tree)
- [IKFoM](https://github.com/hku-mars/IKFoM)

## 🤝 贡献

欢迎提交 Issue 和 Pull Request 来改进本项目！

1. Fork 本仓库
2. 创建您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 Pull Request

## 📄 许可证

本项目基于 MIT 许可证，详情请参阅 [LICENSE](LICENSE) 文件。

## 🙏 致谢

- 感谢 [HKU-MARS](https://github.com/hku-mars) 团队开发的 FAST-LIO2 算法
- 感谢 [Livox](https://www.livoxtech.com/) 提供的 MID360 激光雷达和 SDK
- 感谢所有为开源社区做出贡献的开发者

---

**🌟 如果你觉得这个项目有用，请给它一个 Star！**
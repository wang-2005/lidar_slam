#include <memory>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"

using namespace std::chrono_literals;

class PoseMonitor : public rclcpp::Node {
public:
    PoseMonitor() : Node("pose_monitor") {
        // 初始化 TF 缓存和监听器
        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

        // 设置定时器，每 1 秒触发一次 (1Hz)
        timer_ = this->create_wall_timer(1000ms, std::bind(&PoseMonitor::timer_callback, this));
        
        RCLCPP_INFO(this->get_logger(), "C++ 位姿监视器已启动，正在监听 map -> lidar...");
    }

private:
    void timer_callback() {
        try {
            // 获取从 map 到 lidar 的坐标变换
            geometry_msgs::msg::TransformStamped t;
            t = tf_buffer_->lookupTransform("map", "lidar", tf2::TimePointZero);

            // 1. 提取时间戳
            double timestamp = t.header.stamp.sec + t.header.stamp.nanosec * 1e-9;

            // 2. 提取平移 (Translation)
            double tx = t.transform.translation.x;
            double ty = t.transform.translation.y;
            double tz = t.transform.translation.z;

            // 3. 提取旋转 (Quaternion)
            tf2::Quaternion q(
                t.transform.rotation.x,
                t.transform.rotation.y,
                t.transform.rotation.z,
                t.transform.rotation.w
            );

            // 4. 计算 RPY (弧度)
            tf2::Matrix3x3 m(q);
            double roll, pitch, yaw;
            m.getRPY(roll, pitch, yaw);

            // 打印输出格式化
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "At time " << timestamp << std::endl;
            std::cout << "- Translation: [" << tx << ", " << ty << ", " << tz << "]" << std::endl;
            std::cout << "- Rotation: in Quaternion (xyzw) [" 
                      << q.x() << ", " << q.y() << ", " << q.z() << ", " << q.w() << "]" << std::endl;
            std::cout << "- Rotation: in RPY (radian) [" << roll << ", " << pitch << ", " << yaw << "]" << std::endl;
            std::cout << "- Rotation: in RPY (degree) [" 
                      << roll * 180.0 / M_PI << ", " << pitch * 180.0 / M_PI << ", " << yaw * 180.0 / M_PI << "]" << std::endl;
            
            // 5. 打印 4x4 矩阵
            std::cout << "- Matrix:" << std::endl;
            for (int i = 0; i < 3; i++) {
                tf2::Vector3 row = m.getRow(i);
                double trans_val = (i == 0) ? tx : (i == 1 ? ty : tz);
                std::cout << "  " << row.x() << "  " << row.y() << "  " << row.z() << "  " << trans_val << std::endl;
            }
            std::cout << "  0.000  0.000  0.000  1.000" << std::endl;
            std::cout << "----------------------------------------" << std::endl;

        } catch (const tf2::TransformException & ex) {
            RCLCPP_WARN(this->get_logger(), "无法获取变换: %s", ex.what());
        }
    }

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PoseMonitor>());
    rclcpp::shutdown();
    return 0;
}

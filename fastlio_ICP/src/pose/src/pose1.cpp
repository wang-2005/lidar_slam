#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

class SimplePoseMonitor : public rclcpp::Node {
public:
    SimplePoseMonitor() : Node("pose_monitor") {
        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        tf_buffer_->setUsingDedicatedThread(true);
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

        // 100ms 刷新一次
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&SimplePoseMonitor::get_current_pose, this));

        RCLCPP_INFO(this->get_logger(), "实时位置+航向监控节点启动，监听 map -> body...");
    }

private:
    void get_current_pose() {
        try {
            rclcpp::Time now = this->get_clock()->now() - rclcpp::Duration::from_seconds(0.1);
            auto trans = tf_buffer_->lookupTransform(
                "map", "body", now, rclcpp::Duration::from_seconds(0.5));

            // 位置：米 → 厘米
            double x = trans.transform.translation.x * 100.0;
            double y = trans.transform.translation.y * 100.0;
            double z = trans.transform.translation.z * 100.0;

            // 四元数转欧拉角 → 航向角 yaw（角度制）
            tf2::Quaternion q(
                trans.transform.rotation.x,
                trans.transform.rotation.y,
                trans.transform.rotation.z,
                trans.transform.rotation.w);
            tf2::Matrix3x3 m(q);
            double roll, pitch, yaw;
            m.getRPY(roll, pitch, yaw);
            double yaw_deg = yaw * 180.0 / M_PI;

            // ✅ 关键：用 RCLCPP_INFO 输出，launch里可见！
            RCLCPP_INFO(this->get_logger(), 
                        "位置: X=%.2f cm | Y=%.2f cm | Z=%.2f cm | 偏转Yaw=%.1f°",
                        x, y, z, yaw_deg);

        } catch (const tf2::TransformException & ex) {
            // 静默，不刷屏
        }
    }

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimplePoseMonitor>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

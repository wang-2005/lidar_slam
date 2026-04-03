#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <serial/serial.h>

extern "C" {
    #include "protocol_chassis.h" 
}

// 确保字节对齐，与下位机 STM32 保持一致
#pragma pack(push, 1)
struct ChassisSpeedPayload {
    float vx;
    float vy;
    float wz;
};
#pragma pack(pop)

class SerialBridge : public rclcpp::Node {
public:
    SerialBridge() : Node("serial_bridge_node") {
        try {
            sp_.setPort("/dev/ttyUSB0");
            sp_.setBaudrate(115200);
            serial::Timeout to = serial::Timeout::simpleTimeout(1000);
            sp_.setTimeout(to);
            sp_.open();
        } catch (serial::IOException& e) {
            RCLCPP_ERROR(this->get_logger(), "无法打开串口: %s", e.what());
        }

        if (sp_.isOpen()) {
            RCLCPP_INFO(this->get_logger(), "底盘通讯串口已成功启动");
        }

        sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10, std::bind(&SerialBridge::cmd_callback, this, std::placeholders::_1));
    }

private:
    void cmd_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
        uint8_t buf[MAX_DATA_LENGTH + 10]; 
        
        ChassisSpeedPayload data;
        data.vx = static_cast<float>(msg->linear.x);
        data.vy = static_cast<float>(msg->linear.y);
        data.wz = static_cast<float>(msg->angular.z);

        static uint8_t seq = 0; 
        // 补全被截断的调用
        uint16_t len = Protocol_PackFrame(
            CMD_CHASSIS_SPEED, 
            reinterpret_cast<uint8_t*>(&data), 
            sizeof(data), 
            seq++, 
            buf
        );

        if (sp_.isOpen()) {
            sp_.write(buf, len);
        }
    }

    serial::Serial sp_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SerialBridge>());
    rclcpp::shutdown();
    return 0;
}
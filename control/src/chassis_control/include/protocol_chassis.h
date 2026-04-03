/**
 * @file protocol_chassis.h
 * @brief 底盘上下位机通信协议（解耦合版本）
 * @version 1.0
 * @date 2026-01-22
 */

#ifndef __PROTOCOL_CHASSIS_H
#define __PROTOCOL_CHASSIS_H

#ifdef __cplusplus
extern "C" {
#endif

// #include "main.h"
#include <stdint.h>

/* 协议定义 */
#define FRAME_HEADER_SOF    0xA5        // 帧头标识
#define MAX_DATA_LENGTH     512         // 最大数据长度

/* 命令ID定义 */
#define CMD_CHASSIS_SPEED       0x0101  // 底盘速度控制
#define CMD_CHASSIS_STOP        0x0102  // 底盘急停
#define CMD_HEARTBEAT           0x0FFF  // 心跳包

#define CMD_CHASSIS_FEEDBACK    0x8101  // 底盘状态反馈
#define CMD_ERROR_REPORT        0xFE00  // 错误报告

/* 数据结构定义 */
#pragma pack(1)

// 帧头结构
typedef struct {
    uint8_t sof;            // 帧头标识 0xA5
    uint16_t data_length;   // 数据长度
    uint8_t seq;            // 序列号
    uint8_t crc8;           // 帧头CRC8
} FrameHeader;

// 底盘速度控制
typedef struct {
    float vx;       // X方向速度 (m/s)
    float vy;       // Y方向速度 (m/s)
    float wz;       // 旋转角速度 (rad/s)
} ChassisSpeedCmd;

// 底盘状态反馈
typedef struct {
    uint32_t timestamp;     // 时间戳
    float velocity_x;       // X速度
    float velocity_y;       // Y速度
    float angular_vel;      // 角速度
    float motor_speed[4];   // 电机速度
    uint8_t status;         // 状态
    uint8_t reserved[3];    // 保留
} ChassisFeedback;

// 心跳包
typedef struct {
    uint32_t timestamp;     // 时间戳
    uint8_t alive;          // 存活标志
} Heartbeat;

// 错误报告
typedef struct {
    uint32_t timestamp;     // 时间戳
    uint16_t error_code;    // 错误代码
    uint8_t device_id;      // 设备ID
    uint8_t severity;       // 严重程度
} ErrorReport;

#pragma pack()

/* CRC校验函数 */
uint8_t CRC8_Calculate(const uint8_t *data, uint16_t length);
uint16_t CRC16_Calculate(const uint8_t *data, uint16_t length);

/* 协议编解码函数 */
uint16_t Protocol_PackFrame(uint16_t cmd_id, const uint8_t *data,
                           uint16_t data_len, uint8_t seq, uint8_t *out_buffer);
uint8_t Protocol_ParseFrame(const uint8_t *buffer, uint16_t length,
                           uint16_t *cmd_id, uint8_t **data, uint16_t *data_len);

/* 命令处理回调函数类型 */
typedef void (*Protocol_ChassisSpeedCallback)(float vx, float vy, float wz);
typedef void (*Protocol_ChassisStopCallback)(void);
typedef void (*Protocol_HeartbeatCallback)(void);

/* 协议回调结构体 */
typedef struct {
    Protocol_ChassisSpeedCallback on_chassis_speed;
    Protocol_ChassisStopCallback on_chassis_stop;
    Protocol_HeartbeatCallback on_heartbeat;
} Protocol_Callbacks;

/* 协议处理函数 */
void Protocol_Init(void);
void Protocol_RegisterCallbacks(const Protocol_Callbacks *callbacks);
void Protocol_ProcessData(uint8_t *buffer, uint16_t length);
uint16_t Protocol_PackChassisFeedback(uint32_t timestamp, float vx, float vy, float wz, 
                                     float motor_speed[4], uint8_t status, uint8_t *out_buffer);
uint16_t Protocol_PackErrorReport(uint32_t timestamp, uint16_t error_code, 
                                  uint8_t device_id, uint8_t severity, uint8_t *out_buffer);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_CHASSIS_H */

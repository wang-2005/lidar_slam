/**
 * @file protocol_chassis.c
 * @brief 底盘上下位机通信协议实现（解耦合版本）
 * @version 1.0
 * @date 2026-01-22
 */

#include "protocol_chassis.h"
#include <string.h>

/* 私有变量 */
static uint8_t tx_seq = 0;
static Protocol_Callbacks protocol_callbacks = {0};

/**
 * @brief 计算CRC8校验值
 */
uint8_t CRC8_Calculate(const uint8_t *data, uint16_t length)
{
    uint8_t crc = 0xFF;
    
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }
    
    return crc;
}

/**
 * @brief 计算CRC16校验值
 */
uint16_t CRC16_Calculate(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    
    return crc;
}

/**
 * @brief 打包数据帧
 */
uint16_t Protocol_PackFrame(uint16_t cmd_id, const uint8_t *data,
                           uint16_t data_len, uint8_t seq, uint8_t *out_buffer)
{
    FrameHeader *header = (FrameHeader*)out_buffer;
    uint16_t payload_len = 2 + data_len;  // cmd_id(2) + data
    
    // 1. 填充帧头
    header->sof = FRAME_HEADER_SOF;
    header->data_length = payload_len;
    header->seq = seq;
    
    // 2. 计算帧头CRC8 (对 data_length + seq 共3字节)
    header->crc8 = CRC8_Calculate((uint8_t*)&header->data_length, 3);
    
    // 3. 填充命令ID (小端序)
    out_buffer[5] = cmd_id & 0xFF;
    out_buffer[6] = (cmd_id >> 8) & 0xFF;
    
    // 4. 拷贝数据段
    if (data_len > 0 && data != NULL)
    {
        memcpy(&out_buffer[7], data, data_len);
    }
    
    // 5. 计算整帧CRC16
    uint16_t total_len = 7 + data_len;
    uint16_t crc16 = CRC16_Calculate(out_buffer, total_len);
    out_buffer[total_len] = crc16 & 0xFF;
    out_buffer[total_len + 1] = (crc16 >> 8) & 0xFF;
    
    return total_len + 2;
}

/**
 * @brief 解析数据帧
 */
uint8_t Protocol_ParseFrame(const uint8_t *buffer, uint16_t length,
                           uint16_t *cmd_id, uint8_t **data, uint16_t *data_len)
{
    // 1. 检查最小长度
    if (length < 9) return 0;
    
    // 2. 检查帧头标志
    if (buffer[0] != FRAME_HEADER_SOF) return 0;
    
    FrameHeader *header = (FrameHeader*)buffer;
    
    // 3. 验证帧头CRC8
    uint8_t crc8_calc = CRC8_Calculate((uint8_t*)&header->data_length, 3);
    if (crc8_calc != header->crc8) return 0;
    
    // 4. 检查长度
    uint16_t expected_len = 5 + header->data_length + 2;
    if (length < expected_len) return 0;
    
    // 5. 验证整帧CRC16
    uint16_t crc16_calc = CRC16_Calculate(buffer, expected_len - 2);
    uint16_t crc16_recv = buffer[expected_len - 2] | (buffer[expected_len - 1] << 8);
    if (crc16_calc != crc16_recv) return 0;
    
    // 6. 提取命令ID
    *cmd_id = buffer[5] | (buffer[6] << 8);
    
    // 7. 提取数据段
    *data_len = header->data_length - 2;
    if (*data_len > 0)
    {
        *data = (uint8_t*)&buffer[7];
    }
    else
    {
        *data = NULL;
    }
    
    return 1;
}

/**
 * @brief 协议初始化
 */
void Protocol_Init(void)
{
    tx_seq = 0;
    memset(&protocol_callbacks, 0, sizeof(protocol_callbacks));
}

/**
 * @brief 注册回调函数
 */
void Protocol_RegisterCallbacks(const Protocol_Callbacks *callbacks)
{
    if (callbacks != NULL)
    {
        protocol_callbacks = *callbacks;
    }
}

/**
 * @brief 处理接收到的命令
 */
static void Protocol_ProcessCommand(uint16_t cmd_id, uint8_t *data, uint16_t data_len)
{
    switch (cmd_id)
    {
        case CMD_CHASSIS_SPEED:
        {
            if (data_len == sizeof(ChassisSpeedCmd) && protocol_callbacks.on_chassis_speed != NULL)
            {
                ChassisSpeedCmd *cmd = (ChassisSpeedCmd*)data;
                protocol_callbacks.on_chassis_speed(cmd->vx, cmd->vy, cmd->wz);
            }
            break;
        }
        
        case CMD_CHASSIS_STOP:
        {
            if (protocol_callbacks.on_chassis_stop != NULL)
            {
                protocol_callbacks.on_chassis_stop();
            }
            break;
        }
        
        case CMD_HEARTBEAT:
        {
            if (protocol_callbacks.on_heartbeat != NULL)
            {
                protocol_callbacks.on_heartbeat();
            }
            break;
        }
        
        default:
            break;
    }
}

/**
 * @brief 打包底盘反馈数据
 */
uint16_t Protocol_PackChassisFeedback(uint32_t timestamp, float vx, float vy, float wz, 
                                     float motor_speed[4], uint8_t status, uint8_t *out_buffer)
{
    ChassisFeedback feedback;
    
    feedback.timestamp = timestamp;
    feedback.velocity_x = vx;
    feedback.velocity_y = vy;
    feedback.angular_vel = wz;
    
    for (int i = 0; i < 4; i++)
    {
        feedback.motor_speed[i] = motor_speed[i];
    }
    
    feedback.status = status;
    memset(feedback.reserved, 0, sizeof(feedback.reserved));
    
    return Protocol_PackFrame(CMD_CHASSIS_FEEDBACK,
                             (uint8_t*)&feedback,
                             sizeof(ChassisFeedback),
                             tx_seq++,
                             out_buffer);
}

/**
 * @brief 打包错误报告
 */
uint16_t Protocol_PackErrorReport(uint32_t timestamp, uint16_t error_code, 
                                  uint8_t device_id, uint8_t severity, uint8_t *out_buffer)
{
    ErrorReport error;
    
    error.timestamp = timestamp;
    error.error_code = error_code;
    error.device_id = device_id;
    error.severity = severity;
    
    return Protocol_PackFrame(CMD_ERROR_REPORT,
                             (uint8_t*)&error,
                             sizeof(ErrorReport),
                             tx_seq++,
                             out_buffer);
}

/**
 * @brief 处理接收到的数据（由UART回调调用）
 * @param buffer 接收缓冲区
 * @param length 接收长度
 */

void Protocol_ProcessData(uint8_t *buffer, uint16_t length)
{
    // 查找帧头
    for (uint16_t i = 0; i < length; i++)
    {
        if (buffer[i] == FRAME_HEADER_SOF && (length - i) >= 9)
        {
            // 可能是帧头，尝试解析
            FrameHeader *header = (FrameHeader*)&buffer[i];
            uint16_t frame_len = 5 + header->data_length + 2;
            
            if ((length - i) >= frame_len)
            {
                // 数据足够，尝试解析
                uint16_t cmd_id;
                uint8_t *data;
                uint16_t data_len;
                
                if (Protocol_ParseFrame(&buffer[i], frame_len, &cmd_id, &data, &data_len))
                {
                    Protocol_ProcessCommand(cmd_id, data, data_len);
                    i += frame_len - 1;  // 跳过已处理的帧
                }
            }
            else
            {
                // 数据不足，等待下次接收
                break;
            }
        }
    }
}
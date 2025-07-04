#ifndef __ICM2060__
#define __ICM2060__
#include "Pin_Config.h"

typedef enum ICM2060_STATE{
    ICM2060_OK = 0,
    ICM2060_ERROR,
    ICM2060_INIT_ERROR,
    ICM2060_READ_ERROR,
    ICM2060_WRITE_ERROR,
}ICM2060_STATE;

typedef struct {
    float  temp;              //温度数据
    float  roll, pitch, yaw;  //解算后的欧拉角,加速度计与陀螺仪融合解算中对偏航角并没有什么作用,仅仅是辅助计算其他两角
    float real_yaw;           //实际航向角
}FLINAL_DATA;

extern volatile FLINAL_DATA ICM_EndData;
#define halfT 0.010/2.0f  //计算周期的一半，单位s,10ms

#define ICM_ADDRESS        0x68  // ICM20607设备的I2C地址
#define ICM20607_ID        0x05  // ICM20607设备的ID

#define XG_OFFS_TC_H       0x04  // X轴陀螺仪温度补偿高位
#define XG_OFFS_TC_L       0x05  // X轴陀螺仪温度补偿低位
#define YG_OFFS_TC_H       0x07  // Y轴陀螺仪温度补偿高位
#define YG_OFFS_TC_L       0x08  // Y轴陀螺仪温度补偿低位
#define ZG_OFFS_TC_H       0x0A  // Z轴陀螺仪温度补偿高位
#define ZG_OFFS_TC_L       0x0B  // Z轴陀螺仪温度补偿低位
#define SELF_TEST_X_ACCEL  0x0D  // X轴加速度计自检
#define SELF_TEST_Y_ACCEL  0x0E  // Y轴加速度计自检
#define SELF_TEST_Z_ACCEL  0x0F  // Z轴加速度计自检
#define XG_OFFS_USRH       0x13  // X轴陀螺仪偏移高位
#define XG_OFFS_USRL       0x14  // X轴陀螺仪偏移低位
#define YG_OFFS_USRH       0x15  // Y轴陀螺仪偏移高位
#define YG_OFFS_USRL       0x16  // Y轴陀螺仪偏移低位
#define ZG_OFFS_USRH       0x17  // Z轴陀螺仪偏移高位
#define ZG_OFFS_USRL       0x18  // Z轴陀螺仪偏移低位
#define SMPLRT_DIV         0x19  // 采样率分频器
#define CONFIG             0x1A  // 配置寄存器
#define GYRO_CONFIG        0x1B  // 陀螺仪配置
#define ACCEL_CONFIG       0x1C  // 加速度计配置
#define ACCEL_CONFIG_2     0x1D  // 加速度计配置2
#define LP_MODE_CFG        0x1E  // 低功耗模式配置
#define ACCEL_WOM_X_THR    0x20  // X轴加速度唤醒运动阈值
#define ACCEL_WOM_Y_THR    0x21  // Y轴加速度唤醒运动阈值
#define ACCEL_WOM_Z_THR    0x22  // Z轴加速度唤醒运动阈值
#define FIFO_EN            0x23  // FIFO使能
#define FSYNC_INT          0x36  // 帧同步中断
#define INT_PIN_CFG        0x37  // 中断引脚配置
#define INT_ENABLE         0x38  // 中断使能
#define FIFO_WM_INT_STATUS 0x39  // FIFO水位中断状态
#define INT_STATUS         0x3A  // 中断状态
#define ACCEL_XOUT_H       0x3B  // X轴加速度输出高位
#define ACCEL_XOUT_L       0x3C  // X轴加速度输出低位
#define ACCEL_YOUT_H       0x3D  // Y轴加速度输出高位
#define ACCEL_YOUT_L       0x3E  // Y轴加速度输出低位
#define ACCEL_ZOUT_H       0x3F  // Z轴加速度输出高位
#define ACCEL_ZOUT_L       0x40  // Z轴加速度输出低位
#define TEMP_OUT_H         0x41  // 温度输出高位
#define TEMP_OUT_L         0x42  // 温度输出低位
#define GYRO_XOUT_H        0x43  // X轴陀螺仪输出高位
#define GYRO_XOUT_L        0x44  // X轴陀螺仪输出低位
#define GYRO_YOUT_H        0x45  // Y轴陀螺仪输出高位
#define GYRO_YOUT_L        0x46  // Y轴陀螺仪输出低位
#define GYRO_ZOUT_H        0x47  // Z轴陀螺仪输出高位
#define GYRO_ZOUT_L        0x48  // Z轴陀螺仪输出低位
#define SELF_TEST_X_GYRO   0x50  // X轴陀螺仪自检
#define SELF_TEST_Y_GYRO   0x51  // Y轴陀螺仪自检
#define SELF_TEST_Z_GYRO   0x52  // Z轴陀螺仪自检
#define FIFO_WM_TH1        0x60  // FIFO水位阈值1
#define FIFO_WM_TH2        0x61  // FIFO水位阈值2
#define SIGNAL_PATH_RESET  0x68  // 信号路径复位
#define ACCEL_INTEL_CTRL   0x69  // 加速度智能控制
#define USER_CTRL          0x6A  // 用户控制
#define ICM_PWR_MGMT_1     0x6B  // 电源管理1
#define ICM_PWR_MGMT_2     0x6C  // 电源管理2
#define I2C_IF             0x70  // I2C接口
#define FIFO_COUNTH        0x72  // FIFO计数高位
#define FIFO_COUNTL        0x73  // FIFO计数低位
#define FIFO_R_W           0x74  // FIFO读写
#define WHO_AM_I           0x75  // 设备标识信息
#define XA_OFFSET_H        0x77  // X轴加速度偏移高位
#define XA_OFFSET_L        0x78  // X轴加速度偏移低位
#define YA_OFFSET_H        0x7A  // Y轴加速度偏移高位
#define YA_OFFSET_L        0x7B  // Y轴加速度偏移低位
#define ZA_OFFSET_H        0x7D  // Z轴加速度偏移高位
#define ZA_OFFSET_L        0x7E  // Z轴加速度偏移低位


// int16_t GetRawGyroscopeX(void);
// int16_t GetRawGyroscopeY(void);
// int16_t GetRawGyroscopeZ(void);

// int16_t GetRawAccelerationX(void);
// int16_t GetRawAccelerationY(void);
// int16_t GetRawAccelerationZ(void);

ICM2060_STATE ICM2060_FULL_Init();
ICM2060_STATE ICM2060_Quick_Init(void);
ICM2060_STATE ICM2060_GeOriginalData(void);
void ICM2060_PosSlove(void);
void Printf_ICM2060_OriginalData(void);
void Printf_ICM2060_EndData(float* kp,float* ki,float* kd);
int16_t convertAngle(int16_t angle);
// void Printf_ICM2060_MeanData(void);
#endif
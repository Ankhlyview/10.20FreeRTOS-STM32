#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"
#include "i2c.h"         

/**
 * @brief MPU6050的位置和MPU6050寄存器的位置
 * 
 */
#define MPU6050_ADDR          (0x68 << 1)
#define MPU6050_WHO_AM_I      0x75
#define MPU6050_PWR_MGMT_1    0x6B
#define MPU6050_SMPLRT_DIV    0x19
#define MPU6050_CONFIG        0x1A
#define MPU6050_GYRO_CONFIG   0x1B
#define MPU6050_ACCEL_CONFIG  0x1C
#define MPU6050_ACCEL_XOUT_H  0x3B
#define MPU6050_TEMP_OUT_H    0x41
#define MPU6050_GYRO_XOUT_H   0x43

typedef struct {
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    float   Temperature;
    float   Ax, Ay, Az;     // 单位: g
    float   Gx, Gy, Gz;     // 单位: deg/s
		float   yaw,pitch,roll;
} MPU6050_t;

typedef struct{
	float   yaw,pitch,roll;
} Deal_data_t;

uint8_t MPU6050_Init(void);
void MPU6050_ReadRegs(uint8_t reg, uint8_t *buf, uint16_t len);
void MPU6050_Proc(MPU6050_t *data);
void MPU6050_Read_All(MPU6050_t *data);
void MPU6050_StructureInit(MPU6050_t*data);

#endif 

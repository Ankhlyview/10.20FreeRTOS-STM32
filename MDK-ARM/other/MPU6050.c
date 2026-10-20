#include "mpu6050.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"
extern I2C_HandleTypeDef hi2c2;  

/**
 * @brief 包装HAL库的IIC的写函数，使得传入的参数减少
 * @param[in] reg MPU写入寄存器的位置，data 写入的数据
 */
static void MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, reg,
                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

/**
 * @brief 包装HAL库的IIC的读函数，使得传入的参数减少
 * @param[in] reg MPU写入寄存器的位置，buf接受的位置，len连续读取的长度
 */
void MPU6050_ReadRegs(uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, reg,
                     I2C_MEMADD_SIZE_8BIT, buf, len, 100);
}

/**
 * @brief 初始化MPU6050
 *
 */
uint8_t MPU6050_Init(void)
{
    uint8_t check;

    /* 1. 读 WHO_AM_I 确认设备存在 */
    MPU6050_ReadRegs(MPU6050_WHO_AM_I, &check, 1);
    if (check != 0x68) {
        return 1;   
    }

    /* 2. 解除休眠，选择 PLL 作为时钟源 */
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);


    /* 3. 采样率分频：1kHz / (1 + 7) = 125Hz */
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07);

    /* 4. 配置 DLPF，带宽约 44Hz */
    MPU6050_WriteReg(MPU6050_CONFIG, 0x03);

    /* 5. 陀螺仪量程 ±500°/s (FS_SEL = 1) */
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x08);

    /* 6. 加速度量程 ±4g (AFS_SEL = 1) */
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x08);

    return 0;   // 初始化成功
}

/**
 * @brief 不调用FreeRTOS时，MPU6050的结算方法函数
 * @param[in] 一个结构体，定义在MPU6050.h文件
 */
void MPU6050_Proc(MPU6050_t *data)
{
	static uint32_t nxt = 0;
	if(HAL_GetTick() < nxt) return;
	MPU6050_Read_All(data);
	// 陀螺仪的运算方式
	data->yaw = data->yaw + data->Gz*0.005;
	data->pitch = 0.95238*(data->pitch + data->Gx*0.005)+(1-0.95238)*atan2(data->Ay,data->Az)/3.1415927*180.0f;
	data->roll = 0.95238*(data->roll + data->Gy*0.005)+(1-0.95238)*atan2(data->Ax,data->Az)/3.1415927*180.0f;
	nxt+=5;
}

/**
 * @brief 不调用FreeRTOS读取MPu6050的所有的数值并作简单的处理函数
 * 
 */
void MPU6050_Read_All(MPU6050_t *data)
{
    uint8_t buf[14];


    MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, buf, 14);


    data->Accel_X_RAW = (int16_t)(buf[0] << 8 | buf[1]);
    data->Accel_Y_RAW = (int16_t)(buf[2] << 8 | buf[3]);
    data->Accel_Z_RAW = (int16_t)(buf[4] << 8 | buf[5]);

    data->Temperature = (float)((int16_t)(buf[6] << 8 | buf[7])) / 340.0f + 36.53f;

    data->Gyro_X_RAW  = (int16_t)(buf[8]  << 8 | buf[9]);
    data->Gyro_Y_RAW  = (int16_t)(buf[10] << 8 | buf[11]);
    data->Gyro_Z_RAW  = (int16_t)(buf[12] << 8 | buf[13]);

    data->Ax = data->Accel_X_RAW / 8192.0f;
    data->Ay = data->Accel_Y_RAW / 8192.0f;
    data->Az = data->Accel_Z_RAW / 8192.0f;

    data->Gx = data->Gyro_X_RAW / 65.5f;
    data->Gy = data->Gyro_Y_RAW / 65.5f;
    data->Gz = data->Gyro_Z_RAW / 65.5f;
}

/**
 * @brief  给MPU6050_t 结构体的初始化函数
 * 
 */
void MPU6050_StructureInit(MPU6050_t *data)
{
	data->pitch = 0.0;
	data->roll = 0.0;
	data ->yaw = 0.0;
}



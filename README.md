### 简介
使用FreeRTOS实现基于MPU6050的姿态结算和串口与电脑通信的同时运行
### 硬件清单
STM32C8T6一个，MPU6050一个，0.96寸OLED屏幕一个，，ST-Linkv2一个，USBtoTTL一个
### 接线情况
TTL的TXD-----PA3，TTL的RXD----PA2，OLED的SCL----PB8，OLED的SDA----PB9，MPU6050的SDA----PB11，MPU6050的SCL----PB10，所有的GDN共地
### CubeMx的配置
将PB8,PB9设置为输出引脚，打开I2C2，UART2，同时将系统时钟设置为TIM4
### 结构说明
./MDK-ARM/other中是OLED和MPU6050的模块代码，./MDK-ARM/FreeRTOS中是FreeRTOS的系统移植
### MPU6050姿态解算
使用一阶互补滤波算法进行姿态解算

/*
 * servo.c
 *
 *  Created on: 2019年2月30日
 *      Author: zengwangfa
 *      Notes:  传感器设备【深度传感器、CPU、ADC电压】
 */
 
 #define LOG_TAG    "sensor"
#include "sensor.h"
#include <rtthread.h>
#include <elog.h>
#include "drv_MS5837.h"
#include "gyroscope.h"
#include "stdio.h"
#include "drv_i2c.h"
#include "drv_adc.h"
#include "drv_cpu_temp.h"
#include "drv_cpuusage.h"
extern struct rt_event init_event; /* ALL_init 事件控制块 */


/**
  * @brief  sensor_lowSpeed_thread_entry(低速获取传感器任务函数)
  * @param  void* parameter
  * @retval None
  * @notice 
  */
void sensor_lowSpeed_thread_entry(void* parameter)
{
		uint8 cpu_usage_major, cpu_usage_minor; //整数位、小数位
	
		rt_thread_mdelay(1500);//等待1.5s系统稳定再获取数据
		Sensor.MS5837.PessureValue = get_ms5837_init_pressure(); //获取压力初值

		while(1)
		{
			
				Sensor.CPU.Temperature = get_cpu_temp(); //获取CPU温度
				Sensor.PowerSource.Voltage = get_voltage_value();//获取电源电压值
			
				cpu_usage_get(&cpu_usage_major, &cpu_usage_minor); //获取CPU使用率
				Sensor.CPU.Usage = cpu_usage_major + (float)cpu_usage_minor/100;
			
				rt_thread_mdelay(100);
		}
}

/**
  * @brief  sensor_highSpeed_thread_entry(高速获取传感器任务函数)
  * @param  void* parameter
  * @retval None
  * @notice 
  */
void sensor_highSpeed_thread_entry(void* parameter)
{
		uint8 cpu_usage_major, cpu_usage_minor; //整数位、小数位
	
		rt_thread_mdelay(1500);//等待1.5s系统稳定再获取数据
		Sensor.MS5837.PessureValue = get_ms5837_init_pressure(); //获取压力初值

		while(1)
		{
			
			
				MS5837_Convert();   //MS5837设备数据转换
				JY901_Convert(&Sensor.JY901); //JY901数据转换
				rt_thread_mdelay(10);
		}
}



/*******************************线程初始化*******************************************/
int sensor_thread_init(void)
{
    rt_thread_t sensor_lowSpeed_tid; //低速获取的传感器放入接口
	  rt_thread_t sensor_highSpeed_tid;//高速获取的传感器放入接口
		/*创建动态线程*/
    sensor_lowSpeed_tid = rt_thread_create("sensor",  //线程名称
                    sensor_lowSpeed_thread_entry,		 //线程入口函数【entry】
                    RT_NULL,							   //线程入口函数参数【parameter】
                    1024,										 //线程栈大小，单位是字节【byte】
                    25,										 	 //线程优先级【priority】
                    10);										 //线程的时间片大小【tick】= 100ms

	  sensor_highSpeed_tid = rt_thread_create("sensor",  //线程名称
                    sensor_highSpeed_thread_entry,		 //线程入口函数【entry】
                    RT_NULL,							   //线程入口函数参数【parameter】
                    1024,										 //线程栈大小，单位是字节【byte】
                    25,										 	 //线程优先级【priority】
                    10);										 //线程的时间片大小【tick】= 100ms

    if (sensor_lowSpeed_tid != RT_NULL && sensor_highSpeed_tid != RT_NULL){
				if(MS5837_Init()){
						log_i("MS5837_Init()");
						//rt_event_send(&init_event, MS5837_EVENT);
				}
				else {
						log_e("MS5837_Init_Failed!");
				}
				
				if(adc_init())
				{
						log_i("Adc_Init()");
				}
				rt_thread_startup(sensor_lowSpeed_tid);
				rt_thread_startup(sensor_highSpeed_tid);
		}
		return 0;
}
INIT_APP_EXPORT(sensor_thread_init);


void MS5837_Convert(void)//MS5837数据转换
{
		MS583703BA_getTemperature();//获取外部温度
		MS583703BA_getPressure();   //获取水压

		Sensor.MS5837.Temperature  = get_ms5837_temperature();
		Sensor.MS5837.PessureValue = get_ms5837_pressure();
		Sensor.Depth = (int)((int)(Sensor.MS5837.PessureValue - Sensor.MS5837.Init_PessureValue)/10);
}



/* Get Pressure */
void print_pressure(void)
{
		static char str[50] = {0};


		MS583703BA_getTemperature();//获取温度
		MS583703BA_getPressure();   //获取大气压
	
		sprintf(str,"MS_Temp2:%f\n",MS5837_Temperature);
		rt_kprintf(str);
		rt_kprintf("MS_Pressure:%d\n",MS5837_Pressure);//MS5837_Pressure
}
MSH_CMD_EXPORT(print_pressure, printf pressure[pa]);













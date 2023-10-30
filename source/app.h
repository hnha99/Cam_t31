#ifndef __APP_H__
#define __APP_H__

#include <imp_common.h>
#include <imp_osd.h>
#include <imp_framesource.h>
#include <imp_isp.h>
#include <imp_encoder.h>

#define SENSOR_GC2053

#if defined SENSOR_GC2053
#define SENSOR_NAME "gc2053"
#define SENSOR_CUBS_TYPE TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR 0x37
#define SENSOR_WIDTH 1920
#define SENSOR_HEIGHT 1080
#define CHN0_EN 1
#define CHN1_EN 1
#define CHN2_EN 0
#define CHN3_EN 0
#define CROP_EN 1

#elif defined SENSOR_GC4653
#define SENSOR_NAME "gc4653"
#define SENSOR_CUBS_TYPE TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR 0x29
#define SENSOR_WIDTH 1920
#define SENSOR_HEIGHT 1080
#define CHN0_EN 1
#define CHN1_EN 1
#define CHN2_EN 1
#define CHN3_EN 1
#define CROP_EN 1

#endif

#define MAX_STREAM_NUM 	2
#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_2 2
#define CHANNEL_3 3
#define CHANNEL_ENABLE 1
#define CHANNEL_DISABLE 0

enum RESOLUTION_E
{
	RESOLUTION_VGA,
	RESOLUTION_720P,
	RESOLUTION_1080P,
	RESOLUTION_NONE,
};


#endif // __APP_H__
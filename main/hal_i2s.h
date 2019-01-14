/*
 * hal_i2s.h
 *
 *  Created on: Jul 10, 2018
 *      Author: hai_dotcom
 */

#ifndef MAIN_HAL_I2S_H_
#define MAIN_HAL_I2S_H_
#include "driver/i2s.h"

void hal_i2s_init(uint8_t i2s_num,uint32_t rate,uint8_t bits,uint8_t ch);
int hal_i2s_read(uint8_t i2s_num,char* dest,size_t size,TickType_t timeout);
int hal_i2s_write(uint8_t i2s_num,char* dest,size_t size,TickType_t timeout);


#endif /* MAIN_HAL_I2S_H_ */

/*
 * my_sd_card.h
 *
 *  Created on: Jun 14, 2018
 *      Author: hai_dotcom
 */

#ifndef MAIN_MY_SD_CARD_H_
#define MAIN_MY_SD_CARD_H_

/*Declare library*/
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

/*Prototype*/
void sd_card_init();


#endif /* MAIN_MY_SD_CARD_H_ */

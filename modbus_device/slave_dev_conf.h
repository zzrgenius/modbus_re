/*
 * slave_dev_conf.h
 *
 *  Created on: Dec 21, 2021
 *      Author: zirun
 */

#ifndef SLAVE_DEV_CONF_H_
#define SLAVE_DEV_CONF_H_
#include <unistd.h>
#include <stdio.h>
 #include <linux/kernel.h>
#include <stdint.h>
#define MAX_HOLD_REG_SIZE 100
typedef struct {
	uint16_t id;
    uint16_t reg_addr;
    char name[32];
    float fvalue;
} HoldReg;
typedef struct {
    uint8_t slave_addr;
    char name[32];
    HoldReg *HoldRegs;
} Modbus_Dev;
int get_device_map(void);

#endif /* SLAVE_DEV_CONF_H_ */

/*
 * load_dev.c
 *
 *  Created on: Dec 21, 2021
 *      Author: zirun
 */


#include "cJSON/cJSON.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <io.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/kernel.h>
#include <stdint.h>
#include <errno.h>
#include "slave_dev_conf.h"
#include "modbus/modbus.h"
#include <semaphore.h>
#include <pthread.h>

extern  modbus_mapping_t *mb_mapping;
extern pthread_mutex_t mb_map_mutex;

int get_device_map(void)
{
	uint8_t *p_json_buf = NULL;
	cJSON *json = NULL;
	cJSON *node = NULL;
	cJSON *node_HoldRegs = NULL;
	cJSON *slave_id = NULL;
	cJSON *slave_name = NULL;
	cJSON *slave_regaddr = NULL;
	cJSON *slave_value = NULL;
	char *json_data = NULL;
	int reg_array_size =0;
	HoldReg * pHoldRegs =  NULL;
	FILE *fp = fopen("slave_dev.json", "r");
	if(fp == NULL)
	{
		printf("fopen slave_dev.json: %s\n", strerror(errno));
		return -1;
	}
	 fseek(fp, 0, SEEK_END);
	 int  len = ftell(fp);
//  int  len = filelength(fp);

 	 p_json_buf =  malloc(len+1);
	 if(p_json_buf == NULL)
	 {
		 return -2;
	 }
	 fseek(fp, 0, SEEK_SET);
	 fread(p_json_buf,len,1,fp);
	 fclose(fp);

//	 fflush(stdout);
//	 printf("%s\r",p_json_buf);
	    json = cJSON_Parse((const char*)p_json_buf);
	    if(json == NULL)
	     {
	         printf("parse fail.\n");
	         return -1;
	     }
	    json_data = cJSON_Print(json);
	    printf("data: %s\n", json_data);

	    node = cJSON_GetObjectItem(json,"slave_address");

	    if(node == NULL)
	    {
	        printf("no slave_address \r\n");
	    }
	    else
	    {
	    	if (node->type == cJSON_Int)
	    	{
	    		printf("slave_address:%lld ok\n",node->valueint);
	    	}
	    }
	    free(json_data);
	    node_HoldRegs = cJSON_GetObjectItem(json,"HoldingReg");
	    if(node_HoldRegs == NULL)
	    {
	    	goto to_exit;
	    }
	    reg_array_size = cJSON_GetArraySize(node_HoldRegs);
		printf("reg_array_size:%d \n",reg_array_size);
	    pHoldRegs = malloc(sizeof(HoldReg)*reg_array_size);
	    for(int i = 0; i < reg_array_size;i++)
	    {
	        node = cJSON_GetArrayItem(node_HoldRegs, i);
	        slave_id=cJSON_GetObjectItem(node,"id");
	        if(slave_id != NULL)
	        {
	        	pHoldRegs[i].id = slave_id->valueint;
	        }
	        slave_name=cJSON_GetObjectItem(node,"name");
	        if(slave_name != NULL)
	        {
 	        	strcpy(pHoldRegs[i].name , slave_name->valuestring);
	        }
	        slave_regaddr=cJSON_GetObjectItem(node,"regaddr");
	        if(slave_regaddr != NULL)
	        {
	        	pHoldRegs[i].reg_addr = slave_regaddr->valueint;
	        }
	        slave_value=cJSON_GetObjectItem(node,"value");
	        if(slave_value != NULL)
	        {
	        	pHoldRegs[i].fvalue = slave_value->valuedouble;
	        }
	        printf("id:%d name:%s reg:%d value:%f\r\n",pHoldRegs[i].id,pHoldRegs[i].name ,pHoldRegs[i].reg_addr,pHoldRegs[i].fvalue);
			pthread_mutex_lock(&mb_map_mutex);

//	    	modbus_set_float_abcd(pHoldRegs[i].fvalue, &mb_mapping->tab_registers[2*i+1]);
			modbus_set_float_dcba(pHoldRegs[i].fvalue, &mb_mapping->tab_registers[2*i+1]);

			pthread_mutex_unlock(&mb_map_mutex);

//	    	cJSON *slave_id = NULL;
//	    	cJSON *slave_name = NULL;
//	    	cJSON *slave_regaddr = NULL;
//	    	cJSON *slave_value = NULL;
	    }

	    to_exit:
	    cJSON_Delete(json);
	   sleep(10);
	 return 0;

}

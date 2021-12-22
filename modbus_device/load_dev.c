/*
 * load_dev.c
 *
 *  Created on: Dec 21, 2021
 *      Author: zirun
 */


#include <cjson/cJSON.h>
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
int get_device_map(void)
{
	uint8_t *p_json_buf = NULL;
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
//	 fflush(stdout);
	 printf("%s\r",p_json_buf);
	 fclose(fp);

	 return 0;

}

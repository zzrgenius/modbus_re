/*
 * main.c
 *
 *  Created on: Dec 20, 2021
 *      Author: zirun
 */



#include "modbus/modbus.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#define SLAVE_DEVICE_ID 1
static modbus_mapping_t *mb_mapping;

#if 0
//简单打印信息，定时器触发函数
void print_info(int signo)
{
//	static uint16_t i = 0;
	time_t timep;
	timep = time(NULL);
//	if(mb_mapping == NULL)
//		return ;
//	mb_mapping->tab_registers[2] = i++;
    printf("timer fired:%ld\n",timep);
}

void init_sigaction(){
    struct sigaction act;
    act.sa_handler = print_info;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPROF,&act,NULL); //设置信号 SIGPROF 的处理函数为 print_info

}


void init_time() {
    struct itimerval value;
    value.it_value.tv_sec=2; //定时器启动后，每隔2秒将执行相应的函数
    value.it_value.tv_usec=0;
    value.it_interval=value.it_value;
    setitimer(ITIMER_PROF,&value,NULL); //初始化 timer，到期发送 SIGPROF 信号
}
#endif

const float a1 =       795.5;
const float b1 =        4415;
const float c1 =       316.2;
const float a2 =        3814;
const float b2 =        3604;
const float c2 =        2359;
const float a3 =      -622.8;
const float b3 =        6183;
const float c3 =       206.6;
float generate_pv_power(float x)
{
	float pv_power = 0.0;
	pv_power = a1*exp( -pow( (x-b1)/c1, 2 ) )+ a2*exp( -pow( (x-b2)/c2 ,2 ) )+a3*exp( -pow( (x-b3)/c3,2));
	return pv_power;

}
void timer_thread(union sigval v)
{
	static uint16_t i = 0;
	if(mb_mapping == NULL)
		return ;
	mb_mapping->tab_registers[2] = generate_pv_power(i);

//    printf("timer_thread function! %d\n", v.sival_int);
}
int main(int argc, char *argv[])
{
	timer_t timerid;
	struct sigevent evp;
	struct itimerspec it;

	modbus_t *ctx;
	if(argc > 0)
	{
		if(argv[1] != NULL)
		{
			printf("%s \r\n",argv[1]);
		}
	}
//    init_sigaction();
//    init_time();
	ctx = modbus_new_rtu("/dev/ttyUSB0",9600,'N',8,1);
	modbus_set_slave(ctx, SLAVE_DEVICE_ID);
	modbus_set_debug(ctx, TRUE);
	if (modbus_connect(ctx) == -1)
	{
		fprintf(stderr, "Connection failed:%s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
//	(int nb_bits, int nb_input_bits,int nb_registers, int nb_input_registers);
	mb_mapping = modbus_mapping_new(8, 8, 100, 100);
	if (mb_mapping == NULL)
	{
		fprintf(stderr, "Error mapping: %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	mb_mapping->tab_registers[0] = 1;
	mb_mapping->tab_registers[1] = 2;
	memset(&evp, 0, sizeof(struct sigevent));       //清零初始化
	evp.sigev_value.sival_int = 111;                //也是标识定时器的，回调函数可以获得
	evp.sigev_notify = SIGEV_THREAD;                //线程通知的方式，派驻新线程
	evp.sigev_notify_function = timer_thread;       //线程函数地址
	if(timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
	{
		perror("fail to timer_create");
		return -1;
	}
	/* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
	it.it_interval.tv_sec = 2;  // 回调函数执行频率为1s运行1次
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 3;     // 倒计时3秒开始调用回调函数
	it.it_value.tv_nsec = 0;
	if (timer_settime(timerid, 0, &it, NULL) == -1)
	{
		perror("fail to timer_settime");
		return -1;
	}
//	while(1);
	for (;;)
	{
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
		int rc;

		rc = modbus_receive(ctx, query);
		if (rc > 0)
		{
			modbus_reply(ctx, query, rc, mb_mapping);
		}
		else
		{
			printf("Connection Closed\n");
		}
	}
	return 0;
}

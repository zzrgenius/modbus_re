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
#include <linux/kernel.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <semaphore.h>
#include "zlog.h"
#include <unistd.h>
#include "slave_dev_conf.h"

#define SLAVE_DEVICE_ID 1
static modbus_mapping_t *mb_mapping;
//static sem_t map_sem;
static pthread_mutex_t mb_map_mutex;
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
	float current_pv_power;
	float *fp_data = NULL;
 	uint8_t *p_ucdata = NULL;
	if(mb_mapping == NULL)
		return ;

	pthread_mutex_lock(&mb_map_mutex);
	current_pv_power = generate_pv_power(i++);
	fp_data = &current_pv_power;
 	p_ucdata =	 (uint8_t *)fp_data;

//	mb_mapping->tab_registers[3] = p_udata[0];
//	mb_mapping->tab_registers[2] = p_udata[1];
	modbus_set_float_abcd(*fp_data, &mb_mapping->tab_registers[2]);
//	dzlog_info("\r\n %f data[0]:0x-%02x %02x %02x %02x 0x%04x 0x%04x\r\n",*fp_data,p_ucdata[0],p_ucdata[1],p_ucdata[2],p_ucdata[3],mb_mapping->tab_registers[2],mb_mapping->tab_registers[3]);
	pthread_mutex_unlock(&mb_map_mutex);

//	  sem_post(&map_sem);

//    printf("timer_thread function! %d\n", v.sival_int);
}
extern int letter_config(void);

int  init_for_modbus(void)
{
	int rc = 0;
//	rc = dzlog_init("test_default.conf", "my_cat");
//	if (rc) {
//		dzlog_warn("dzlog init failed\n");
//	}
	rc = get_device_map();
//	if (rc) {
//		dzlog_warn("get_device_map  failed\n");
//
//	}
 	pthread_mutex_init(&mb_map_mutex, NULL);
 	letter_config();
 	return rc;
}
extern int json_main(void);

static void mytest(void)
{
	unsigned int x=0x12345678; /* 305419896 */

	unsigned char *p=(unsigned char *)&x;
	printf(" %02x %02x %02x %02x\r\n",p[0],p[1],p[2],p[3]);
	fflush(stdout);
	json_main();
	return ;
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
	sleep(1);
	mytest();

//    init_sigaction();
//    init_time();
//	sem_init(&map_sem, 0, 1);
	init_for_modbus();
	ctx = modbus_new_rtu("/dev/ttyUSB0",9600,'N',8,1);
	modbus_set_slave(ctx, SLAVE_DEVICE_ID);
//	modbus_set_debug(ctx, TRUE);  //打开modbus调试
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
	it.it_interval.tv_sec = 1;  // 回调函数执行频率为1s运行1次
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 5;     // 倒计时3秒开始调用回调函数
	it.it_value.tv_nsec = 0;
	if (timer_settime(timerid, 0, &it, NULL) == -1)
	{
		perror("fail to timer_settime");
		return -1;
	}
//	while(1);
//	dzlog_info("goto loop\r");
	for (;;)
	{
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
		int rc;

		rc = modbus_receive(ctx, query);
		if (rc > 0)
		{
//			  sem_wait(&map_sem);
			pthread_mutex_lock(&mb_map_mutex);

			modbus_reply(ctx, query, rc, mb_mapping);
			pthread_mutex_unlock(&mb_map_mutex);

		}
		else
		{
			printf("Connection Closed\n");
		}
	}
	zlog_fini();
	modbus_mapping_free(mb_mapping);
	return 0;
}

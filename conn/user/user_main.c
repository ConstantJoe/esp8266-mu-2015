/*
Useful Resources:
Set of Documents that show how to do lots of basic things: http://bbs.espressif.com/viewforum.php?f=21
USB-to-TTL Serial Cable description: http://www.adafruit.com/product/954
ESP8266 WiFi Module Quick Start Guide: http://rancidbacon.com/files/kiwicon8/ESP8266_WiFi_Module_Quick_Start_Guide_v_1.0.4.pdf
ESP8266-EVB description and docs: https://www.olimex.com/Products/IoT/ESP8266-EVB/
ESP8266 Programming Guide: http://hackaday.com/2015/03/18/how-to-directly-program-an-inexpensive-esp8266-wifi-module/
*/

#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"


#include "os_type.h"
#include "user_interface.h"
#include "mem.h"
#include "ip_addr.h"
#include "espconn.h" 

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

LOCAL struct espconn *pCon = NULL;

//extern uint8_t at_wifiMode;
//extern void user_esp_platform_load_param(void *param, uint16 len);

static volatile os_timer_t some_timer;

const char ssid[32] = "Joe";
const char password[32] = "password";

struct station_config stationConf;

static void ICACHE_FLASH_ATTR scbfunc(void *arg)
{
	//uart0_sendStr("got in send callback");	
}

static void ICACHE_FLASH_ATTR rcbfunc(void *arg, char*pdata, unsigned short len)
{
	uart0_sendStr("got in recv callback");
	//char s[len+1];	
	//s[0] = pdata + '0';
		
	uart0_sendStr(pdata);	
}

static void ICACHE_FLASH_ATTR send_request(void *arg)
{


  	struct espconn *pespconn = (struct espconn *)arg;

  	char payload[128];

  	os_sprintf(payload, "GET /remotepi/pilog.php?sid=0000000000000001&ts=260615.145400&event=TEST\r\n");
	
	espconn_regist_sentcb(pespconn, scbfunc);
	espconn_regist_recvcb(pespconn, rcbfunc);
	
  	espconn_sent(pespconn, payload, strlen(payload));
  
}

//Do nothing function
//The OS requires a main loop to be assigned to take place after the init. Since I only want things to run using a timer, I've created this main loop but made it do effectively nothing.
static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events)
{
    	os_delay_us(10);
}

 void some_timerfunc(void *arg)
{
	uart0_sendStr("\r\nIn the timer!\r\n");

	//if not connected correctly aleady, connect to the wifi
	if(wifi_station_get_connect_status() != 5)
	{
		wifi_set_opmode( 0x01 );
		os_memcpy(&stationConf.ssid, ssid, 32);
		os_memcpy(&stationConf.password, password, 32);
		wifi_station_set_config(&stationConf);
		wifi_station_connect();

		char str[10];
		str[0] = wifi_station_get_connect_status() +'0';
		uart0_sendStr("start wifi connect status");	
		uart0_sendStr(str);	
		uart0_sendStr("finish wifi connect status");	
	}
	else
	{
		pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
		if (pCon == NULL)
		{
	        	os_printf("pCon ALLOCATION FAIL\r\n");	
 	        	return;
		}

		pCon->type = ESPCONN_TCP;
		pCon->state = ESPCONN_NONE;
    
		pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
		pCon->proto.tcp->local_port = espconn_port();
		//set up the server remote port
		pCon->proto.tcp->remote_port = 80;

		//set up the remote IP
		uint32_t ip = ipaddr_addr("149.157.248.82"); //IP address for sbsrv1.cs.nuim.ie
		os_memcpy(pCon->proto.tcp->remote_ip, &ip, 4);

		//set up the local IP
		struct ip_info ipconfig;
		wifi_get_ip_info(STATION_IF, &ipconfig);
		os_memcpy(pCon->proto.tcp->local_ip, &ipconfig.ip, 4);

		int ret = 0;
		ret = espconn_connect(pCon);

		if(ret == 0){ 
			//uart0_sendStr("\r\n%d\r\n",ret);
			uart0_sendStr("\r\nespconn_connect OK!\r\n");

			espconn_regist_connectcb(pCon, send_request);
		}
		else	
		{
      			//INFO("espconn_connect FAILED!\r\n");  
			uart0_sendStr("\r\nespconn_connect failed!\r\n");
	      		char *fail;
      			os_sprintf(fail, "%d \r\n", ret);
      			//clean up allocated memory
      			if(pCon->proto.tcp) os_free(pCon->proto.tcp);
      			os_free(pCon);
      			pCon = NULL;
		}
	}
	
}

void ICACHE_FLASH_ATTR user_init(void)
{
	//Start up uart0 -> this bit rate is the default on the device
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	
//Disarm timer
	os_timer_disarm(&some_timer);

	//Setup timer
	os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

	//Arm the timer, &some_timer is the pointer 10000 is the fire time in ms
	//0 for once and 1 for repeating timer
 	os_timer_arm(&some_timer, 10000, 1);

	 
  	//uart_init(BIT_RATE_115200, BIT_RATE_115200);
  	/*uart0_sendStr("\r\nready test\r\n");
  	at_init();*/
	//Start os task
	system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

}

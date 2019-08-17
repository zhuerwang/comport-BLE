#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <stdlib.h>
#include "ble.h"
#include "comport.h"
#include "cJSON.h"
#include "get_time.h"

#define DEVNAME "/dev/ttyUSB0"



int             g_stop = 0;

void sig_handler(int sig_t);
void sig_handler(int sig_t)
{
    if(SIGKILL==sig_t)
        g_stop=1;
}


int main(int argc, char *argv[])
{
    int         rc = -1;
    int         rv = -1;
    int         ret = -1;
    int         opt = 0;
    int         baudrate = 9600;
    char        *setting = "8N1N";
	cJSON 		*cjson;
    char        val_buf[512];
	char 		cjson_str[512];
	char 		time_buf[128];
    char        *zErrMsg;	
	sqlite3 	*db;
	char 		sql[256];

    ble_ctx_t   ble;
    comport_info_t *comport;

    struct option opts[]=
    {
        {"baudrate" ,required_argument  ,NULL   ,'b'},
        {"setting"  ,required_argument  ,NULL   ,'s'},
        {"help"     ,no_argument        ,NULL   ,'h'},
    };

    while((opt = getopt_long(argc, argv, "b:s:h", opts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'b':
                baudrate = atoi(optarg);
                break;
            case 's':
                setting = optarg;
                break;
            case 'h':
                printf("usage:--b [baudrate] --s [databit parity stopbit flowctl] -h [help]\n");
                exit(1);
                break;
            defult:
                break;
        }
    }

    signal(SIGKILL, sig_handler);

    comport = comport_init(DEVNAME , baudrate, setting);
    ble.comport = comport;

    if((rv = ble_init(&ble)) < 0)
    {
        printf("ble device initialize failed.\n");
        goto error;
    }
    printf("ble device initialize successful.\n");

    printf("BLE Dongle Name: %s \n", ble.blename);
    printf("BLE Dongle Addr: %s \n", ble.bleaddr);

    if((rv = database_init(db, zErrMsg)) < 0)
    {
        printf("database initialize failed.\n");
        goto sqlite3_error;
    }
    printf("database initialize successful.\n");

    printf("BLE is waiting for connecting........\n");
	while( !g_stop )
	{
        if('0' == ble.connected) /* while ble lost connect and try connect each 1s */
        {
            if(ble_sleep(&ble))
            {
                printf("Ble is sleep mode and wait for connecting......\n");
            }

            while((rv = ble_check_conn(&ble))< 0)
            {
                sleep(1);
                continue;
            }
        }
        else    /* while ble has been connecting and check if the ble lost connect */
        {
            
        }

		if((ret = ble_recv_data(&ble, cjson_str, sizeof(cjson_str))) < 0)
		{
			printf("Read json string failed.\n");

            if((ret = ble_reply(&ble)) < 0)
            {
                printf("Reply master failed.\n");
            }
		}
		else
		{
            printf("%s\n",cjson_str);
			if((ret = ble_reply(&ble)) < 0)
			{
				printf("Reply master failed.\n");
			}
		}
        
        if(ble.data_ok == '1')
        {
            
	        printf("-------------gather temperature------------\n");
            
		    cjson = cJSON_Parse(cjson_str);	
		    if(cjson)
		    {
			    printf("SN : %s\n", cJSON_GetObjectItem(cjson, "SN")->valuestring);
			    printf("temperature : %s\n", cJSON_GetObjectItem(cjson, "temperature")->valuestring);
			    printf("humidity : %s%\n", cJSON_GetObjectItem(cjson, "humidity")->valuestring);
			    printf("real time : %s\n", get_sys_time(time_buf ,sizeof(time_buf)));
		    }   
		    else
		    {
			    printf("Get json object failed.\n");
		    }

		    snprintf(sql,sizeof(sql),"INSERT INTO STUDENT (NUMBER,SN,TEMPER,HUMIDITY,TIME) VALUES (NULL,'%s','%s','%s','%s');",
						    cJSON_GetObjectItem(cjson, "SN")->valuestring,
						    cJSON_GetObjectItem(cjson, "temperature")->valuestring,
						    cJSON_GetObjectItem(cjson, "humidity")->valuestring,
						    time_buf);
		    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
		    if(rc != SQLITE_OK)
		    {
			    fprintf(stderr, "SQL error: %s\n", zErrMsg);
			    sqlite3_free(zErrMsg);
		    }
		
            printf("Insert SQL successfull.\n");
		    cJSON_Delete(cjson);	

		    printf("---------------------------------------\n");
        }
    }

	sqlite3_close(db);
    comport_close(comport);
	return 0;


sqlite3_error:
	sqlite3_close(db);

error:
    comport_close(comport);
	return -1;

}

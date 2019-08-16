#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include "ble.h"
#include "comport.h"
#include "cJSON.h"
#include "get_time.h"

#define DEVNAME "/dev/ttyUSB0"

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

    comport_info_t *ci;

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

    ci = comport_init(DEVNAME , baudrate, setting);

    ble_wake(ci);

    if((rv = ble_enquire_name(ci, val_buf, sizeof(val_buf))) < 0)
    {
        printf("BLE get device name failed.\n");
        goto error;
    }
    printf("BLE name : %s\n",val_buf);
    

    if((rv = ble_enquire_addr(ci, val_buf, sizeof(val_buf))) < 0)
    {
        printf("BLE get device MAC address failed.\n");
        goto error;
    }
    printf("BLE addr : %s\n",val_buf);

    if((rv = ble_set_slave_role(ci)) < 0)
    {
        printf("Set ble as slave mode failed.\n");
        goto error;
    }
    else
    {
        printf("BLE set slave mode successful.\n");
    }


    printf("BLE is waiting for connecting........\n");
    while(!(ci->conn_bit))
    {    
        if((rv = ble_check_conn(ci)) < 0)
        {
            ble_sleep(ci);
        }
    }

	rv = sqlite3_open("ble.db", &db);
	if( rv )
	{	
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        goto error;
	}
	else
	{
		fprintf(stdout, "Opened database successfully.\n");
	}

	char *sql_s =   "CREATE TABLE IF NOT EXISTS STUDENT(" \
			 "NUMBER  INTEGER PRIMARY KEY     AUTOINCREMENT,"\
			 "SN      CHAR(64)    NOT NULL,"\
			 "TEMPER    CHAR(64)    NOT NULL,"\
			 "HUMIDITY  CHAR(64)    NOT NULL,"
             "TIME CHAR(64)   NOT NULL);";		   

	rc = sqlite3_exec(db, sql_s, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		goto sqlite3_error;
	}
	else
	{
		fprintf(stdout, "Table create successfully\n");
	}

	printf("-------------gather temperature------------\n");
	/* while datapack coming then gather temperature and humidity */
	while(ci->conn_bit)
	{

		if((ret = ble_recv_data(ci, cjson_str, sizeof(cjson_str))) < 0)
		{
			printf("Read json string failed.\n");

            if((ret = ble_reply(ci)) < 0)
            {
                printf("Reply master failed.\n");
            }
			goto sqlite3_error;
		}
		else
		{
            printf("%s\n",cjson_str);
			if((ret = ble_reply(ci)) < 0)
			{
				printf("Reply master failed.\n");
				goto sqlite3_error;
			}
		}

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
			goto json_error;
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
			goto json_error;
		}
		
        printf("Insert SQL successfull.\n");
		cJSON_Delete(cjson);	

		printf("---------------------------------------\n");
	}

	sqlite3_close(db);
    comport_close(ci);
	return 0;

json_error:
	cJSON_Delete(cjson);

sqlite3_error:
	sqlite3_close(db);

error:
    comport_close(ci);
	return -1;

}

#include <stdio.h>
#include <string.h>
#include "comport.h"
#include "at_cmd.h"
#include "ble.h"

#define SLAVE_OK_REPLY          "SLAVE RECIVE OK"
#define SLAVE_ERROR_REPLY       "SLAVE RECIVE FAIL"
#define ATCMD_LIST_ITEMS        6

#define CONN_OK                 "OK+CONN"

static char *atcmd_list[ATCMD_LIST_ITEMS]=
{
    "AT\r\n",
    "AT+NAME\r\n",
    "AT+LADDR\r\n",
    "AT+ROLE0\r\n",
    "AT+SLEEP\r\n",
    "WAKE\r\n",
};

/* name :ble_init()
 * to do :
 * initialize the member of ble structure
 */
int ble_init(ble_ctx_t *ble)
{
    int         rv = -1;


    if((rv = ble_enquire_name(ble, ble->blename, BLENAME_LEN)) < 0)
    {
        printf("BLE get device name failed.\n");
        return -1;
    }


    if((rv = ble_enquire_addr(ble, ble->bleaddr, BLEADDR_LEN)) < 0)
    {
        printf("BLE get device name failed.\n");
        return -1;
    }
    
    ble_wake(ble);
    
    if((rv = ble_set_slave_role(ble)) < 0)
    {
        printf("Set ble as slave mode failed.\n");
        return -1;
    }


    ble->connected = 0;
        
    return 1;

}

/* name : ble_enquire_name()
 * to do :
 * enquire the ble dongle's name
 */
int ble_enquire_name(ble_ctx_t *ble, char *val_buf, int size)
{
    int             rv = -1;

   // printf("atcmd:%s\n", sizeof(atcmd_list[1]));
    if((rv = at_cmd_enquire(ble->ci, atcmd_list[1], val_buf, strlen(atcmd_list[1]), 1500)) < 0)
    {
        printf("Enquire BLE failed.\n");
        return -1;
    }

    return 1;
}

/* name : ble_enquire_addr()
 * to do :
 * enquire the ble dongle's MAC address
 */
int ble_enquire_addr(ble_ctx_t *ble, char *val_buf,int size)
{
    int             rv = -1;

    if((rv = at_cmd_enquire(ble->ci, atcmd_list[2], val_buf, strlen(atcmd_list[2]), 1500)) < 0)
    {
        printf("Enquire BLE failed.\n");
        return -1;
    }

    return 1;
}

/* name : ble_sleep()
 * to do :
 * BLE go to sleep
 */
int ble_sleep(ble_ctx_t *ble)
{
    int             rv = -1;

    if((rv = at_cmd_set(ble_ctx_t *ble, atcmd_list[4], strlen(atcmd_list[4]), 1500)) < 0)
    {
        printf("Enquire BLE failed.\n");
        return -1;
    }

    return 1;
}


/* name : ble_wake()
 * to do :
 * BLE wake up
 */
int ble_wake(ble_ctx_t *ble)
{
    int             rv = -1;
    int             try = 20;
    int             block_time = 100;
    char            ret_buf[256];

    /* try wake up 20 times */
    while(try--)
    {
        comport_write(ble->ci, atcmd_list[5], strlen(atcmd_list[5]));

        memset(ret_buf, 0, sizeof(ret_buf));
        rv = comport_read(ble->ci, ret_buf, sizeof(ret_buf), 1000, block_time);

        rv = at_check_set(ret_buf);
        if(rv > 0)
        break;
    }
        printf("BLE is wake up\n");

    return 1;
}


/* name : ble_set_slave_role()
 * to do :
 * set the ble dongle's role
 * 0 slave 1 master
 */
int ble_set_slave_role(ble_ctx_t *ble)
{
    int             rv = -1;

    if((rv = at_cmd_set(ble->ci, atcmd_list[3], strlen(atcmd_list[3]), 1500)) < 0)
    {
        return -1;
    }

    return 1;
}

/* name : ble_check_conn()
 * to do : 
 * slave BLE mode and wait for connecting 
 */
int ble_check_conn(ble_ctx_t *ble)
{
    int             rv = -1;
    char            recv_buf[512];

    /* wait ten second */
    memset(recv_buf, 0, sizeof(recv_buf));
    if((rv = comport_read(ble->ci , recv_buf, sizeof(recv_buf), 1000, 1000)) < 0)
    {
        return -1;
    }
    else if(!strstr(recv_buf, CONN_OK))
    {
        printf("BLE connect failed.\n");
        return -2;
    }
   
    ble->connected = 1;
    printf("BLE has been connected.\n");

    return 1;

}


/* name : ble_recv_data()
 * to do :
 * ble as slave recive the data from master and check lost in the same time
 */
int ble_recv_data(ble_ctx_t *ble, char *cjson_str, int size)
{
    int         rv = -1;

    memset(cjson_str, 0, size);
    if((rv = comport_read(ble->ci, cjson_str, size, 5000, 100)) < 0)
    {
        printf("Read data from master failed.\n");
        ble->data_ok = 0;
        return -1;
    }
    else if(strstr(cjson_str, "LOST"))
    {
        ble->connected = 0;
        ble->data_ok = 0;
        return -2;
    }

    ble->data_ok = 1;
    return 1;

}

/* name : ble_reply()
 * to do :
 * if ble recive data from master will reply the result
 */
int ble_reply(ble_ctx_t *ble)
{
    int         rv = -1;

    if(ble->data_ok)
    {
        if((rv = comport_write(ble->ci, "SLAVE RECIVE OK", strlen("SLAVE RECIVE OK"))) < 0)
        {
            printf("Reply master failed.\n");
            return -1;
        }

    }
    else
    {
        if((rv = comport_write(ble->ci, SLAVE_ERROR_REPLY, strlen(SLAVE_ERROR_REPLY))) < 0)
        {
            printf("Reply master failed.\n");
            return -2;
        }
    }

    return 1;
}


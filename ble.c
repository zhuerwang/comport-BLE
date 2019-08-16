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

/* name : ble_enquire_name()
 * to do :
 * enquire the ble dongle's name
 */
int ble_enquire_name(comport_info_t *ci, char *val_buf, int size)
{
    int             rv = -1;

   // printf("atcmd:%s\n", sizeof(atcmd_list[1]));
    if((rv = at_cmd_enquire(ci, atcmd_list[1], val_buf, strlen(atcmd_list[1]), 1500)) < 0)
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
int ble_enquire_addr(comport_info_t *ci, char *val_buf,int size)
{
    int             rv = -1;

    if((rv = at_cmd_enquire(ci, atcmd_list[2], val_buf, strlen(atcmd_list[2]), 1500)) < 0)
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
int ble_sleep(comport_info_t *ci)
{
    int             rv = -1;

    if((rv = at_cmd_set(ci, atcmd_list[4], strlen(atcmd_list[4]), 1500)) < 0)
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
int ble_wake(comport_info_t *ci)
{
    int             rv = -1;
    int             try = 20;
    int             block_time = 100;
    char            ret_buf[256];

    /* try wake up 20 times */
    while(try--)
    {
        comport_write(ci, atcmd_list[5], strlen(atcmd_list[5]));

        memset(ret_buf, 0, sizeof(ret_buf));
        rv = comport_read(ci, ret_buf, sizeof(ret_buf), 1000, block_time);

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
int ble_set_slave_role(comport_info_t *ci)
{
    int             rv = -1;

    if((rv = at_cmd_set(ci, atcmd_list[3], strlen(atcmd_list[3]), 1500)) < 0)
    {
        return -1;
    }

    return 1;
}

/* name : ble_check_conn()
 * to do : 
 * slave BLE block and check the connection atatus
 */
int ble_check_conn(comport_info_t *ci)
{
    int             rv = -1;
    char            recv_buf[512];

    /* wait ten second */
    memset(recv_buf, 0, sizeof(recv_buf));
    if((rv = comport_read(ci , recv_buf, sizeof(recv_buf), 1000, 10000)) < 0)
    {
        printf("No master BLE connect in 10s.\n");
        return -1;
    }
    else if(!strstr(recv_buf, CONN_OK))
    {
        printf("BLE connect failed.\n");
        return -2;
    }
   
    ci->conn_bit = 1;
    printf("BLE has been connected.\n");

    return 1;

}

/* name : ble_recv_data()
 * to do :
 * ble as slave recive the data from master
 */
int ble_recv_data(comport_info_t *ci, char *cjson_str, int size)
{
    int         rv = -1;

    memset(cjson_str, 0, size);
    if((rv = comport_read(ci, cjson_str, size, 5000, 20000)) < 0)
    {
        printf("Read data from master failed.\n");
        ci->data_ok = 0;
        return -1;
    }
    ci->data_ok = 1;
    return 1;

}

/* name : ble_reply()
 * to do :
 * if ble recive data from master will reply the result
 */
int ble_reply(comport_info_t *ci)
{
    int         rv = -1;

    if(ci->data_ok)
    {
        if((rv = comport_write(ci, "SLAVE RECIVE OK", strlen("SLAVE RECIVE OK"))) < 0)
        {
            printf("Reply master failed.\n");
            return -1;
        }

    }
    else
    {
        if((rv = comport_write(ci, SLAVE_ERROR_REPLY, strlen(SLAVE_ERROR_REPLY))) < 0)
        {
            printf("Reply master failed.\n");
            return -2;
        }
    }

    return 1;
}


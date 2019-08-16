#include <stdio.h>
#include <string.h>
#include "comport.h"
#include "at_cmd.h"

/* name : at_cmd_enquire()
 * to do :
 * send some command which will recive value of parameter
 * 
 * example :
 * command[AT+NAME\r\n]   reply[+NAME=CC41-B\r\n]
 */

int at_cmd_enquire(comport_info_t *ci, char *atcmd, char *val_buf, int size, int timeout)
{
    int             rv = -1;
    char            ret_buf[512];

    if(NULL == atcmd)
    {
        printf("Send error atcmd to enquire.\n");
        return -1;
    }

    /* check BLE status */
    if(1 == (ci->conn_bit))
    {
        printf("BLE device is connecting, Don't send AT to BLE device");
        return -2;
    }

    if((rv = comport_write(ci, atcmd, size)) < 0)
    {
        printf("Send AT command failed.\n");
        return rv;
    }

    memset(ret_buf, 0, sizeof(ret_buf));
    if((rv = comport_read(ci, ret_buf, sizeof(ret_buf), timeout, 10000)) < 0)
    {
        printf("Recive AT command failed.\n");
        return rv;
    }
    if(!at_analysis(ret_buf, val_buf))
    {
        printf("Analysis failed.\n");
        return -3;
    }
   
    return 1;
}


/* name : at_cmd_set()
 * to do :
 * send some cmd which can set some value and recive "OK\r\n" or "ERROR\r\n" 
 *
 * example :
 * command[AT+NAME<Param>\r\n]   reply[+NAME=<Param>OK]
 */

int at_cmd_set(comport_info_t *ci, char *atcmd, int size, int timeout)
{
    int             rv = -1;
    char            ret_buf[512];

    if(NULL == atcmd)
    {
        printf("Set error atcmd to set.\n");
        return -1;
    }

    if(1 == (ci->conn_bit))
    {
        printf("BLE device is connecting, Don't send AT to BLE device");
        return -2;
    }

    if((rv = comport_write(ci, atcmd, size))<0)
    {
        printf("Send AT command failed.\n");
        return rv;
    }

    memset(ret_buf, 0, sizeof(ret_buf));
    if((rv = comport_read(ci, ret_buf, sizeof(ret_buf), timeout, 10000)) < 0)
    {
        printf("Recive AT command failed.\n");
        return rv;
    }

    rv = at_check_set(ret_buf);
    if(rv < 0)
    {
        printf("BLE handle atcmd and reply ERROR.\n");
        return rv;
    }
    else if(rv == 0)
    {
        printf("BLE handle atcmd and reply nothing.\n");
        return rv;
    }

    return 1;
}


/* name : at_cmd_exec()
 * to do :
 * send some cmd which will carry out something and only recive "OK\r\n" or "ERROR\r\n"
 *
 * example :
 * command[AT+RESET]   reply["OK\r\n"]
 */

int at_cmd_exec(comport_info_t *ci, char *atcmd, int size, int timeout)
{
    int             rv = -1;
    char            ret_buf[512];

    if(NULL == atcmd)
    {
        printf("Set error atcmd to set.\n");
        return -1;
    }

    if(1 == (ci->conn_bit))
    {
        printf("BLE device is connecting, Don't send AT to BLE device");
        return -2;
    }

    if((rv = comport_write(ci, atcmd, size))<0)
    {
        printf("Send AT command failed.\n");
        return rv;
    }
    
    if((rv = comport_read(ci, ret_buf, sizeof(ret_buf), timeout, 10000)) < 0)
    {
        printf("Recive AT command failed.\n");
        return rv;
    }

    rv = at_check_exec(ret_buf);
    if(rv < 0)
    {
        printf("BLE handle atcmd and reply ERROR.\n");
        return rv;
    }
    else if(rv == 0)
    {
        printf("BLE handle atcmd and reply nothing.\n");
        return rv;
    }

    return 1;
}



/* name : at_check_ok()
 * to do :
 * check the value of at_cmd_exec()
 */

int at_check_exec(char *ret_buf)
{
    if(strstr(ret_buf, "OK"))
    {
        return 1;
    }
    else if(strstr(ret_buf, "ERROR"))
    {
        return -1;
    }
    return 0;
}


/* name : at_check_set()
 * to do :
 * check the value of at_cmd_set() 
 */
at_check_set(char *ret_buf)
{
    if(strstr(ret_buf, "+"))
    {
        return 1;
    }
    else if(strstr(ret_buf, "ERROR"))
    {
        return -1;
    }
    return 0;
}

/* name : at_analysis()
 * to do :
 * analysis value which you want
 */

int at_analysis(char *ret_buf, char *val_buf)
{
    char *start;

    if((start = strchr(ret_buf, '=')) == NULL)
    {
        printf("Analysis '=' charactor in the response command failed.\n");
        return -1;
    }
    
    if(!strncpy(val_buf , start+1, 20))
    {
        printf("Copy value failed.\n");
        return -2;
    }
    
    return 1;
}

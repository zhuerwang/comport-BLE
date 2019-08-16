
#ifndef COM__INFO
#define COM__INFO

typedef struct comport_info{
    int     fd;
    int     baud;
    char    databit;
    char    stopbit;
    char    parity;
    char    flowctl;
    int     conn_bit;
    int     data_ok;
}comport_info_t;


void msleep(int msec);
comport_info_t *comport_init(char *dev_name, int baudrate, char *setting);
int comport_write(comport_info_t *ci, char *send_buf, int size);
int comport_read(comport_info_t *ci, char *recive_buf, int size, int timeout, int block_time);
int comport_set(comport_info_t *ci);
void comport_close(comport_info_t *ci);
#endif

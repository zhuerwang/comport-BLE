#include "comport.h"

#ifndef BLE__H
#define BLE__H


#define BLENAME_LEN          32
#define BLEADDR_LEN          32

typedef struct ble_ctx_s
{
    comport          *comport;
    char             blename[BLENAME_LEN];
    char             bleaddr[BLEADDR_LEN];
    char             connectd; /*0: Not connected  1: Connected */
    char             data_ok; /*0: data error 1: data ok */
}ble_ctx_t;

int ble_init(ble_ctx_t *ble);
int ble_check_conn(ble_ctx_t *ble);
int ble_wake(ble_ctx_t *ble);
int ble_sleep(ble_ctx_t *ble);
int ble_enquire_name(ble_ctx_t *ble, char *val_buf, int size);
int ble_enquire_addr(ble_ctx_t *ble, char *val_buf, int size);
int ble_set_slave_role(ble_ctx_t *ble);
int ble_recv_data(ble_ctx_t *ble, char *cjson_str, int size);
int ble_reply(ble_ctx_t *ble);
#endif

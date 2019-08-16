

#ifndef BLE__H
#define BLE__H

#include "comport.h"

int ble_check_conn(comport_info_t *ci);
int ble_wake(comport_info_t *ci);
int ble_sleep(comport_info_t *ci);
int ble_enquire_name(comport_info_t *ci, char *val_buf, int size);
int ble_enquire_addr(comport_info_t *ci, char *val_buf, int size);
int ble_set_slave_role(comport_info_t *ci);
int ble_recv_data(comport_info_t *ci, char *cjson_str, int size);
int ble_reply(comport_info_t *ci);
#endif

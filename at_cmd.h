

#ifndef AT_CMD_H
#define AT_CMD_H


int at_cmd_enquire(comport_info_t *ci, char *atcmd, char *val_buf, int size, int timeout);
int at_cmd_set(comport_info_t *ci, char *atcmd, int size, int timeout);
int at_cmd_exec(comport_info_t *ci, char *atcmd, int size, int timeout);
int at_check_exec(char *ret_buf);
int at_check_set(char *ret_buf);
int at_analysis(char *ret_buf, char *val_buf);

#endif

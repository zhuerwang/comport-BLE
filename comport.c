#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "comport.h"

comport_info_t* comport_init(char *dev_name, int baudrate, char *setting)
{
    int ret = 0;
    comport_info_t *ci ; 
    
    ci = (comport_info_t *)malloc(sizeof(comport_info_t));

    ci->conn_bit = 0;
    ci->baud = baudrate;
    ci->databit = setting[0];
    ci->parity = setting[1];
    ci->stopbit = setting[2];
    ci->flowctl = setting[3];

    ci->fd = open(dev_name , O_RDWR | O_NOCTTY | O_NONBLOCK);
    if((ci->fd) < 0)
    {   
        printf("At initialize failed:%s\n",strerror(errno));
        return NULL;       
    }

  	if(0 == isatty(STDIN_FILENO))
	{
		printf("Standard input is not a terminal device.\n");
		return NULL;
	}

	if(!(ret = comport_set(ci)))
    {
        printf("comport set option failed.\n");
        return NULL;
    }

    return ci;
}


int comport_write(comport_info_t *ci, char *send_buf, int size)
{
	int rv = -1;

	rv = write(ci->fd, send_buf, size);
	if( rv < 0 )
	{
		printf("Write comport failed:%s\n",strerror(errno));
		return rv;
	}

	return rv;
}

/* made a micro second sleep */
void msleep(int msec)
{
    struct timeval  timeout;

    timeout.tv_sec = msec/1000;
    timeout.tv_usec = msec%1000;

    select(1, NULL, NULL, NULL, &timeout);
};


int comport_read(comport_info_t *ci, char *recive_buf, int size, int timeout, int block_time)
{
    fd_set          rset;
    struct timeval  t_out;
	int             rv = -1;
    
    FD_ZERO(&rset);
    FD_SET(ci->fd, &rset);
    t_out.tv_sec = (time_t)(block_time / 1000);
    t_out.tv_usec = (long)(1000 * (block_time % 1000));
    rv = select(ci->fd+1, &rset, NULL, NULL, &t_out);
    if(rv < 0)
    {
        printf("Select failed:%s\n",strerror(errno));
        return -1;
    }
    else if(rv == 0)
    {
        return -2;
    }

    msleep(timeout); // Max to 15 charactor
    
    memset(recive_buf, 0, size);
	rv = read(ci->fd, recive_buf, size);
	if(rv <= 0)
	{
		printf("Read comport failed or timeout:%s\n",strerror(errno));
		return -3;
	}

    tcflush(ci->fd, TCIFLUSH);

	return 1;
}


int comport_set(comport_info_t *ci)
{

	/* termios is a standard interface that can represent URAT */
	struct termios newtio,oldtio;
    
	if(tcgetattr(ci->fd, &oldtio) < 0)
	{
		printf("Get old termios failed.");
		return 0;
	}

	memset(&newtio, 0, sizeof(newtio));
	/* used to activate local connection and recive */
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    newtio.c_oflag &= ~(OPOST);

	switch(ci->baud)
	{
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
	}
    

	switch(ci->databit)
	{
		case '7':
			newtio.c_cflag |= CS7;
			break;
		case '8':
			newtio.c_cflag |= CS8;
			break;
	}


	switch(ci->parity)
	{
		case 'O':
        case 'o':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_cflag |= (INPCK | ISTRIP);
			break;
		case 'E':
        case 'e':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			newtio.c_cflag |= (INPCK | ISTRIP);
			break;
		case 'N':
        case 'n':
			newtio.c_cflag &= ~PARENB;
			break;
    }
    
    
	switch(ci->stopbit)
	{
		case '1':
			newtio.c_cflag &= ~CSTOPB;
		    break;
		case '2':
			newtio.c_cflag |= CSTOPB;
			break;
	}

    switch (ci->flowctl)
    {
        case 'S':                       // Software control
        case 's':
        case 'B':
        case 'b':
            newtio.c_cflag &= ~(CRTSCTS);
            newtio.c_iflag |= (IXON | IXOFF);
            break;
        case 'H':                       // Hardware control
        case 'h':
            newtio.c_cflag |= CRTSCTS;   // Also called CRTSCTS
            newtio.c_iflag &= ~(IXON | IXOFF);
            break;
        case 'N':
        case 'n':
        default:                 // NONE
            newtio.c_cflag &= ~(CRTSCTS);
            newtio.c_iflag |= (IXON|IXOFF);
            break;
    }

	newtio.c_cc[VTIME] = 0;// timeout before 1s
	newtio.c_cc[VMIN] = 0;
	tcflush(ci->fd,TCIFLUSH);
    
	if(tcsetattr(ci->fd, TCSANOW, &newtio) < 0)
	{

		printf("Set comport failed.\n");
		return 0;
	}

	return 1;
}

void comport_close(comport_info_t *ci)
{
    close(ci->fd);
    free(ci);
}

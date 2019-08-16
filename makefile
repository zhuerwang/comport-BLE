CROSS_COMPILE=/opt/crosstool/cortex-a7/bin/arm-linux-gnueabi-
LIB_PATH=/home/iot/lihaojie/arm_sqlite/lib
INCLUDE_PATH=/home/iot/lihaojie/arm_sqlite/include
export CC=${CROSS_COMPILE}gcc

all:
	@${CC} *.c -o ble -I ${INCLUDE_PATH} -L ${LIB_PATH} -lsqlite3 -lm
	@sz ble

clean:
	@rm -f ble

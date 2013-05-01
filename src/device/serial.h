#ifndef __SERIAL__H__
#define __SERIAL__H__

#include <device/device.h>

typedef enum {
	 BAUD_300    =    300
	,BAUD_600    =    600
	,BAUD_1200   =   1200
	,BAUD_2400   =   2400
	,BAUD_4800   =   4800
	,BAUD_9600   =   9600
	,BAUD_19200  =  19200
	,BAUD_38400  =  38400
	,BAUD_57600  =  57600
	,BAUD_115200 = 115200
} Baud;

typedef enum {
	 DATA_BIT_7 = 7
	,DATA_BIT_8 = 8
} DataBits;

typedef enum {
	 STOP_BIT_1 = 1
	,STOP_BIT_2 = 2
} StopBits;

typedef enum {
	 PARITY_NONE
	,PARITY_EVEN
	,PARITY_ODD
} Parity;

/**
 * Configuration for a serial communication line (RS232).
 */
struct serial_config_t {
	char name[128];
	Baud baud_rate;
	DataBits data_bits;
	StopBits stop_bits;
	Parity parity;
};

extern const struct device_operations_t serial_device_operations;

#endif

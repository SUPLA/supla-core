/*
 ============================================================================
 Name        : gpio.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef GPIO_H_
#define GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_BUFFER_SIZE 50
#define GPIO_MINDELAY    200000

typedef struct {
	#ifdef __GPIO_SIMULATE
	int ifd;
	#endif
	int fd;
	unsigned char number;
	char value;
	void *user_data1;
	void *user_data2;
}TGpioPort;

typedef void (*_func_gpio_portchanged)(TGpioPort *port);

//void *gpio_init(_func_gpio_portchanged on_portchanged);
//void gpio_free(void *_gpio);

void gpio_wait(TGpioPort *port, int count, int usec, _func_gpio_portchanged on_portchanged);
char gpio_set_value(unsigned char port_number, char value);
char gpio_get_value(unsigned char port_number, char *value);
char gpio_port_init(unsigned char port_number, unsigned char in, char value);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */

#include "stdio.h"

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"

#include "light.h"

static void i2c_init(void) {
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	I2C_Init(LPC_I2C2, 100000);

	I2C_Cmd(LPC_I2C2, ENABLE);
}

void init() {
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 5;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(2, 1 << 5, 0);
	LPC_GPIOINT ->IO2IntEnF |= 1 << 5;
	LPC_GPIOINT ->IO2IntClr = 1 << 5;

	NVIC_ClearPendingIRQ(EINT3_IRQn);
	NVIC_SetPriorityGrouping(4);
	NVIC_SetPriority(EINT3_IRQn, NVIC_EncodePriority(4, 2, 0));
	NVIC_EnableIRQ(EINT3_IRQn);

	i2c_init();
	light_enable();

	light_setRange(LIGHT_RANGE_4000);
	light_setHiThreshold(2000);
	light_setLoThreshold(0);
	light_setIrqInCycles(LIGHT_CYCLE_1);

	light_clearIrqStatus();
}

int main(void) {
	init();
	while (1) {
	}
	return 0;
}

void EINT3_IRQHandler(void) {
	if ((LPC_GPIOINT ->IO2IntStatF >> 5) & 0x1) {
		LPC_GPIOINT ->IO2IntClr = 1 << 5;
		light_clearIrqStatus();
		int light = light_read();
		printf("Light ISR: %d\n", light);
	}
	printf("I'm in the ISR\n");
}

void check_failed(uint8_t *file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1) {
	}
}

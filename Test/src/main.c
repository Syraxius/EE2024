/*****************************************************************************
 *   A demo example using several of the peripherals on the base board
 *
 *   Copyright(C) 2011, EE2024
 *   All rights reserved.
 *
 ******************************************************************************/

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "led7seg.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"

#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"
#include "light.h"

volatile uint32_t msTicks; //counter for 1ms sysTicks
int8_t xoff, yoff, zoff, x, y, z;
int32_t temp;
uint32_t light;

//SysTick Handler - just increment SysTick counter
void SysTick_Handler(void) {
	msTicks++;
}

uint32_t getMsTicks() {
	return msTicks;
}

static void init_ssp(void) {
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);
}

static void init_i2c(void) {
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

void init_systick() {
	SysTick_Config(SystemCoreClock / 1000); //1ms Interrupts
}

void init_led7seg() {
	led7seg_init();
	led7seg_setChar('0', FALSE);
}

void init_light() {
	light_enable();
}

void init_temp() {
	temp_init(&getMsTicks);
}

void init_accelerometer() {
	acc_init();
	acc_read(&xoff, &yoff, &zoff);
}

void init_sw3() {

}

void init_oled() {
	oled_init();
	oled_clearScreen(OLED_COLOR_BLACK);
}

void init() {
	init_i2c();
	init_ssp();

	//I2C Devices
	init_light();
	init_accelerometer();

	//GPIO Devices
	init_temp();
	init_sw3();

	//SSP Devices
	init_led7seg();
	init_oled();

	//System Devices
	init_systick();
}

uint8_t hasPassed(uint32_t *lastRun, uint32_t duration) {
	if (getMsTicks() - *lastRun >= duration) {
		*lastRun = getMsTicks();
		return 1;
	} else {
		return 0;
	}
}

void tickTime() {
	static uint32_t lastRun = 0;
	static uint32_t seconds = 0;

	if (hasPassed(&lastRun, 1000)) {
		seconds = (seconds + 1) % 10;
		int light = light_read();
		led7seg_setChar('0' + seconds, FALSE);
	}
}

void readAcc() {
	acc_read(&x, &y, &z);
	x = x - xoff;
	y = y - yoff;
	z = z - zoff;
}

void readTemp() {
	temp = temp_read();
}

void readLight() {
	light = light_read();
}

void readSensors() {
	readAcc();
	readTemp();
	readLight();
}

void printResults() {
	uint8_t text[32];
	sprintf(text, "X: %d     ", x);
	oled_putString(0, 0, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Y: %d     ", y);
	oled_putString(0, 10, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Z: %d     ", z);
	oled_putString(0, 20, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Temp: %d     ", temp);
	oled_putString(0, 30, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Light: %d     ", light);
	oled_putString(0, 40, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
}

void readPrintResults() {
	static uint32_t lastRun = 0;

	if (hasPassed(&lastRun, 3000)) {
		readSensors();
		printResults();
	}
}

void loop() {
	while (1) {
		tickTime();
		readPrintResults();
	}
}

int main(void) {
	init();
	loop();

	return 0;
}

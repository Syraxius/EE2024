#include "stdio.h"

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"

#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"
#include "light.h"
#include "temp.h"
#include "led7seg.h"

#define BASIC 0
#define RESTRICTED 1

int32_t xoff = 0;
int32_t yoff = 0;
int32_t zoff = 0;
int32_t temp = 0;
uint32_t light = 0;
uint32_t msTicks = 0;
int8_t x = 0;
int8_t y = 0;
int8_t z = 0;
uint8_t mode = BASIC;

static void i2c_setup(void) {
	printf("Initializing I2C... ");
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	I2C_Init(LPC_I2C2, 100000);

	I2C_Cmd(LPC_I2C2, ENABLE);
	printf("Success!\n");
}

static void ssp_setup(void) {
	printf("Initializing SSP... ");
	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;
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
	printf("Success!\n");
}

uint32_t getMsTicks(void) {
	return msTicks;
}

void systick_setup() {
	printf("Initializing SysTick... ");
	SysTick_Config(SystemCoreClock / 1000);
	printf("Success!\n");
}

void timer_init(LPC_TIM_TypeDef *TIMx, uint32_t time) {
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;

	// Initialize timer 0, prescale count time of 1ms
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue = 1000;
	// use channel 0, MR0
	TIM_MatchConfigStruct.MatchChannel = 0;
	// Enable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch = TRUE;
	//Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	//Do not stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch = FALSE;
	//Do no thing for external output
	TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	// Set Match value, count value is time (timer * 1000uS =timer mS )
	TIM_MatchConfigStruct.MatchValue = time;
	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(TIMx, TIM_TIMER_MODE, &TIM_ConfigStruct);
	TIM_ConfigMatch(TIMx, &TIM_MatchConfigStruct);
	// To start timer 0
	TIM_Cmd(TIMx, ENABLE);
}

void timers_setup() {
	printf("Initializing Timers... ");
	timer_init(LPC_TIM1, 1000);
	timer_init(LPC_TIM2, 3100);
	timer_init(LPC_TIM3, 250);
	printf("Success!\n");
}

void light_setup() {
	printf("Initializing Light Sensor... ");
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 5;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(2, 1 << 5, 0);

	light_init();
	light_enable();
	light_setRange(LIGHT_RANGE_64000);
	light_setWidth(LIGHT_WIDTH_16BITS);
	light_setIrqInCycles(LIGHT_CYCLE_1);
	light_setHiThreshold(2000);
	light_setLoThreshold(0);

	LPC_GPIOINT ->IO2IntEnF |= 1 << 5;
	light_clearIrqStatus();
	printf("Success!\n");
}

void acc_setup() {
	printf("Initializing Accelerometer... ");
	acc_init();
	acc_read(&x, &y, &z);
	xoff = 0 - x;
	yoff = 0 - y;
	zoff = 0 - z;
	printf("Success!\n");
}

void temp_setup() {
	printf("Initializing Temperature Sensor... ");
	temp_init(&getMsTicks);
	printf("Success!\n");
}

static void sw3_setup(void) {
	printf("Initializing Switch 3... ");
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 10;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(2, 1 << 10, 0);

	LPC_GPIOINT ->IO2IntEnF |= 1 << 10;
	printf("Success!\n");
}

void led7seg_setup() {
	printf("Initializing 7-Segment LED... ");
	led7seg_init();
	led7seg_setChar('0', FALSE);
	printf("Success!\n");
}

void oled_setup() {
	printf("Initializing OLED... ");
	oled_init();
	oled_clearScreen(OLED_COLOR_BLACK);
	printf("Success!\n");
}

void pca9532_setup() {
	printf("Initializing LED Array... ");
	pca9532_init();
	pca9532_setLeds(0xffff, 0xffff);
	printf("Success!\n");
}

void rgb_setup() {
	printf("Initializing RGB LED... ");
	rgb_init();
	rgb_setLeds(RGB_GREEN | RGB_BLUE);
	printf("Success!\n");
}

void uart_setup(void) {
	printf("Initializing UART... ");
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	UART_CFG_Type uartCfg;
	uartCfg.Baud_rate = 115200;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;
	UART_Init(LPC_UART3, &uartCfg);

	UART_TxCmd(LPC_UART3, ENABLE);
	printf("Success!\n");
}

void nvic_setup() {
	printf("Initializing NVIC... ");
	NVIC_SetPriorityGrouping(5);

	uint32_t ans, PG = 5, PP = 0b00, SP = 0b000;
	ans = NVIC_EncodePriority(PG, PP, SP);
	NVIC_SetPriority(SysTick_IRQn, ans);
	NVIC_SetPriority(TIMER3_IRQn, ans);

	PG = 5, PP = 0b01, SP = 0b000;
	ans = NVIC_EncodePriority(PG, PP, SP);
	NVIC_SetPriority(TIMER1_IRQn, ans);

	PG = 5, PP = 0b01, SP = 0b001;
	ans = NVIC_EncodePriority(PG, PP, SP);
	NVIC_SetPriority(TIMER2_IRQn, ans);
	NVIC_SetPriority(EINT3_IRQn, ans);

	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_EnableIRQ(TIMER2_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(UART3_IRQn);
	printf("Success!\n");
}

void init() {
	i2c_setup();
	ssp_setup();

	systick_setup();
	timers_setup();

	acc_setup();
	light_setup();
	temp_setup();

	sw3_setup();

	led7seg_setup();
	oled_setup();
	pca9532_setup();
	rgb_setup();
	uart_setup();

	nvic_setup();
}

void tickTime() {
	static int time = 0;
	led7seg_setChar('0' + time, FALSE);
	time = (time + 1) % 10;
}

void readAccelerometer() {
	acc_read(&x, &y, &z);
	x = x + xoff;
	y = y + yoff;
	z = z + zoff;
}

void readLight() {
	light = light_read();
}

void readTemp() {
	temp = temp_read();
}

void basicRead() {
	readTemp();
	readLight();
	readAccelerometer();
}

void restrictedRead() {
//Handled by Light Interrupt
}

void readSensors() {
	if (mode == BASIC) {
		basicRead();
	} else if (mode == RESTRICTED) {
		restrictedRead();
	}
}

void basicPrint() {
	uint8_t text[32];
	sprintf(text, "X: %d    ", x);
	oled_putString(0, 10, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Y: %d    ", y);
	oled_putString(0, 20, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Z: %d    ", z);
	oled_putString(0, 30, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Light: %d Lux    ", light);
	oled_putString(0, 40, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Temp: %.1f C    ", temp / 10.0);
	oled_putString(0, 50, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	sprintf(text, "Hello World!\r\n");
	//UART_Send(LPC_UART3, &text, strlen(text), BLOCKING);
}

void restrictedPrint() {
	uint8_t text[32];
	strcpy(text, "X: R    ");
	oled_putString(0, 10, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	strcpy(text, "Y: R    ");
	oled_putString(0, 20, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	strcpy(text, "Z: R    ");
	oled_putString(0, 30, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	strcpy(text, "Light: R        ");
	oled_putString(0, 40, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	strcpy(text, "Temp: R        ");
	oled_putString(0, 50, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
}

void printResults() {
	uint8_t text[32];

	if (mode == BASIC) {
		strcpy(text, "BASIC     ");
		basicPrint();
	} else if (mode == RESTRICTED) {
		strcpy(text, "RESTRICTED");
		restrictedPrint();
	}

	oled_putString(0, 0, &text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	Timer0_Wait(1);
}

void switchToBasic() {
	mode = BASIC;
	TIM_ResetCounter(LPC_TIM2 );
	NVIC_DisableIRQ(TIMER3_IRQn);
	NVIC_EnableIRQ(TIMER2_IRQn);
	rgb_setLeds(RGB_GREEN | RGB_BLUE);
}

void switchToRestricted() {
	mode = RESTRICTED;
	NVIC_DisableIRQ(TIMER2_IRQn);
	rgb_setLeds(RGB_GREEN | RGB_RED);
	printResults();
}

void basicUpdate() {
	if (light >= 2000) {
		switchToRestricted();
	}
}

void restrictedUpdate() {
	static uint32_t ledOnMask = 0;

	if (light < 2000) {
		ledOnMask = (ledOnMask << 1 | 0x1) & 0xffff;
	} else {
		ledOnMask = 0;
	}

	pca9532_setLeds(ledOnMask, 0xffff);

	if (ledOnMask == 0xffff) {
		switchToBasic();
	}
}

void updateMode() {
	if (mode == BASIC) {
		basicUpdate();
	} else if (mode == RESTRICTED) {
		restrictedUpdate();
	}
}

void loop() {
	while (1) {
	}
}

int main(void) {
	init();
	loop();
	return 0;
}

void TIMER1_IRQHandler(void) {
	TIM_ClearIntPending(LPC_TIM1, 0);
	tickTime();
}

void TIMER2_IRQHandler(void) {
	TIM_ClearIntPending(LPC_TIM2, 0);
	readSensors();
	printResults();
}

void TIMER3_IRQHandler(void) {
	TIM_ClearIntPending(LPC_TIM3, 0);
	updateMode();
}

void EINT3_IRQHandler(void) {
	if ((LPC_GPIOINT ->IO2IntStatF >> 10) & 0x1) {
		LPC_GPIOINT ->IO2IntClr = 1 << 10;
		basicRead();
		basicPrint();
	}

	if ((LPC_GPIOINT ->IO2IntStatF >> 5) & 0x1) {
		LPC_GPIOINT ->IO2IntClr = 1 << 5;
		light_clearIrqStatus();

		readLight();

		if (light < 2000) {
			light_setLoThreshold(0);
		} else {
			light_setLoThreshold(1999);
		}

		updateMode();

		TIM_ResetCounter(LPC_TIM3 );
		TIM_ClearIntPending(LPC_TIM3, 0);
		NVIC_EnableIRQ(TIMER3_IRQn);
	}
}

void SysTick_Handler(void) {
	msTicks++;
}

void check_failed(uint8_t *file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1) {
	}
}


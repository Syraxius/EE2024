#include "stdio.h"

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"

//I2C
#include "acc.h"
#include "light.h"
#include "pca9532.h"

//SPI
#include "led7seg.h"
#include "oled.h"

//GPIO
#include "rgb.h"
#include "temp.h"

#define BASIC 0
#define RESTRICTED 1

#define UART_SAMPLING_TIME 1
#define UART_FLARE_INTENSITY 2
#define UART_TIME_UNIT 3
#define UART_GET_SETTINGS 4

uint32_t SAMPLING_TIME = 3000;
uint32_t FLARE_INTENSITY = 2000;
uint32_t TIME_UNIT = 250;

int32_t xoff = 0;
int32_t yoff = 0;
int32_t zoff = 0;
int32_t temp = 0;
int32_t tempRiseTime = 0;
int32_t tempDeltaTime = 0;

uint32_t light = 0;
uint32_t usTicks = 0;

int8_t x = 0;
int8_t y = 0;
int8_t z = 0;

uint8_t mode = BASIC;
uint8_t forceBasicReadFlag = FALSE;
uint8_t switchBasicPrintFlag = TRUE;
uint8_t switchRestrictedPrintFlag = FALSE;
uint8_t forceBasicPrintFlag = FALSE;
uint8_t readPrintFlag = FALSE;
uint8_t tickTimeFlag = FALSE;

void enterAtomic() {
	__set_BASEPRI(0b01001000);
}

void exitAtomic() {
	__set_BASEPRI(0b11111111);
}

uint32_t getUsTicks(void) {
	return usTicks;
}

uint32_t getMsTicks() {
	return usTicks / 1000;
}

//I2C

static void i2c_setup(void) {
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

void acc_setup() {
	acc_init();
	acc_read(&x, &y, &z);
	xoff = 0 - x;
	yoff = 0 - y;
	zoff = 0 - z;
}

void light_setup() {
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
	light_setHiThreshold(FLARE_INTENSITY);
	light_setLoThreshold(0);

	LPC_GPIOINT ->IO2IntEnF |= 1 << 5;
	light_clearIrqStatus();
}

void pca9532_setup() {
	pca9532_init();
	pca9532_setLeds(0xffff, 0);
}

//SPI

static void ssp_setup(void) {
	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK1;
	 * P0.8 - MISO1
	 * P0.9 - MOSI1
	 * P2.2 - SSEL1 - used as GPIO
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
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
	SSP_Cmd(LPC_SSP1, ENABLE);
}

void led7seg_setup() {
	led7seg_init();
	led7seg_setChar('0', FALSE);
}

void oled_setup() {
	oled_init();
	oled_clearScreen(OLED_COLOR_BLACK);
}

//GPIO

void rgb_setup() {
	rgb_init();
	rgb_setLeds(RGB_GREEN | RGB_BLUE);
}

static void sw3_setup(void) {
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 10;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(2, 1 << 10, 0);

	LPC_SC ->EXTMODE |= (1 << 0);
	LPC_SC ->EXTPOLAR &= ~(1 << 0);
}

void temp_setup() {
	temp_init(&getUsTicks);
	LPC_GPIOINT ->IO0IntEnF |= (1 << 2);
	LPC_GPIOINT ->IO0IntEnR |= (1 << 2);
}

//System

void systick_setup() {
	SysTick_Config(SystemCoreClock / 1000000);
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
	timer_init(LPC_TIM3, TIME_UNIT);
}

void uart_setup(void) {
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

	LPC_UART3 ->IER |= (1 << 0);
	LPC_UART3 ->FCR |= (0b01000011); // Interrupt every 4bits received.
}

void nvic_setup() {
	uint32_t PG = 5;

	NVIC_SetPriorityGrouping(5);

	uint32_t ans, PP = 0b00, SP = 0b000;
	ans = NVIC_EncodePriority(PG, PP, SP);
	NVIC_SetPriority(SysTick_IRQn, ans);

	ans, PP = 0b00, SP = 0b010;
	NVIC_SetPriority(EINT3_IRQn, ans);

	PP = 0b01, SP = 0b000;
	ans = NVIC_EncodePriority(PG, PP, SP);
	NVIC_SetPriority(TIMER3_IRQn, ans);

	PP = 0b10, SP = 0b000;
	ans = NVIC_EncodePriority(PG, PP, SP);
	NVIC_SetPriority(EINT0_IRQn, ans);
	NVIC_SetPriority(UART3_IRQn, ans);

	NVIC_EnableIRQ(EINT0_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(UART3_IRQn);
}

void init() {
	//I2C
	i2c_setup();
	acc_setup();
	light_setup();
	pca9532_setup();

	//SPI
	ssp_setup();
	led7seg_setup();
	oled_setup();

	//GPIO
	rgb_setup();
	sw3_setup();
	temp_setup();

	//System
	systick_setup();
	timers_setup();
	uart_setup();
	nvic_setup();
}

void tickTime() {
	enterAtomic();

	static int time = 0;
	led7seg_setChar('0' + time, FALSE);
	time = (time + 1) % 10;

	exitAtomic();
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
	//temp = temp_read();
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
	enterAtomic();
	if (mode == BASIC || forceBasicReadFlag) {
		forceBasicReadFlag = FALSE;
		basicRead();
	} else if (mode == RESTRICTED) {
		restrictedRead();
	}
	exitAtomic();
}

void basicPrint() {
	uint8_t text[32];

	if (switchBasicPrintFlag) {
		switchBasicPrintFlag = FALSE;
		sendStartMessage();
		strcpy(text, " BASIC      ");
		oled_putString(0, 0, &text, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	}

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

	sprintf(text, "L%d_T%.1f_AX%d_AY%d_AZ%d\r\n", light, temp / 10.0, x, y, z);
	UART_Send(LPC_UART3, &text, strlen(text), BLOCKING);
}

void restrictedPrint() {
	uint8_t text[32];

	if (switchRestrictedPrintFlag) {
		switchRestrictedPrintFlag = FALSE;
		sendStopMessage();
		strcpy(text, " RESTRICTED ");
		oled_putString(0, 0, &text, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	}

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

void sendStartMessage() {
	char* text =
			"Space Weather is Back to Normal. Scheduled Telemetry Will Now Resume.\r\n";
	UART_Send(LPC_UART3, (uint8_t *) text, strlen(text), BLOCKING);
}

void sendStopMessage() {
	char* text =
			"Solar Flare Detected. Scheduled Telemetry is Temporarily Suspended.\r\n";
	UART_Send(LPC_UART3, (uint8_t *) text, strlen(text), BLOCKING);
}

void printResults() {
	uint8_t text[32];
	enterAtomic();

	if (mode == BASIC || forceBasicPrintFlag || switchBasicPrintFlag) {
		forceBasicPrintFlag = FALSE;
		basicPrint();
	} else if (mode == RESTRICTED && switchRestrictedPrintFlag) {
		restrictedPrint();
	}

	exitAtomic();
}

void switchToBasic() {
	mode = BASIC;
	rgb_setLeds(RGB_GREEN | RGB_BLUE);

	NVIC_DisableIRQ(TIMER3_IRQn);

	switchBasicPrintFlag = TRUE;
	readPrintFlag = TRUE;
}

void switchToRestricted() {
	mode = RESTRICTED;
	rgb_setLeds(RGB_GREEN | RGB_RED);

	TIM_ClearIntPending(LPC_TIM3, 0);
	NVIC_EnableIRQ(TIMER3_IRQn);

	switchRestrictedPrintFlag = TRUE;
	readPrintFlag = TRUE;
}

void basicUpdate() {
	if (light >= FLARE_INTENSITY) {
		switchToRestricted();
	}
}

void restrictedUpdate() {
	static uint32_t ledOnMask = 0;
	static uint32_t lastRun = 0;
	uint8_t shiftNum = (getMsTicks() - lastRun) / TIME_UNIT;
	lastRun = lastRun + shiftNum * TIME_UNIT;

	if (light < FLARE_INTENSITY) {
		while (shiftNum-- > 0) {
			ledOnMask = (ledOnMask << 1 | 0x1) & 0xffff;
		}
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

int getElapsedTimeMs(uint32_t lastRun) {
	uint32_t now = getMsTicks();
	uint32_t elapsedTime;

	if (now >= lastRun) {
		elapsedTime = now - lastRun;
	} else {
		elapsedTime = (0xFFFFFFFF - lastRun + 1) + now;
	}

	return elapsedTime;
}

int getElapsedTimeUs(uint32_t lastRun) {
	uint32_t now = getUsTicks();
	uint32_t elapsedTime;

	if (now >= lastRun) {
		elapsedTime = now - lastRun;
	} else {
		elapsedTime = (0xFFFFFFFF - lastRun + 1) + now;
	}

	return elapsedTime;
}

int hasElapsed(uint32_t *lastRun, uint32_t duration) {
	uint32_t elapsedTime = getElapsedTimeMs(*lastRun);

	if (elapsedTime >= duration) {
		*lastRun = getMsTicks();
		return 1;
	} else {
		return 0;
	}
}

void tickTimeLoop() {
	static uint32_t lastRun = 0;
	if (hasElapsed(&lastRun, 1000)) {
		tickTime();
	}
}

void readPrintLoop() {
	static uint32_t lastRun = 0;
	if (readPrintFlag || hasElapsed(&lastRun, SAMPLING_TIME)) {
		readPrintFlag = FALSE;
		readSensors();
		printResults();
	}
}

void loop() {
	while (1) {
		tickTimeLoop();
		readPrintLoop();
	}
}

void oled_drawStar(int shiftx, int shifty, oled_color_t color) {
	oled_line(0 + shiftx, 20 + shifty, 50 + shiftx, 20 + shifty, color);
	Timer0_Wait(100);
	oled_line(50 + shiftx, 20 + shifty, 10 + shiftx, 50 + shifty, color);
	Timer0_Wait(100);
	oled_line(10 + shiftx, 50 + shifty, 25 + shiftx, 0 + shifty, color);
	Timer0_Wait(100);
	oled_line(25 + shiftx, 0 + shifty, 40 + shiftx, 50 + shifty, color);
	Timer0_Wait(100);
	oled_line(40 + shiftx, 50 + shifty, 0 + shiftx, 20 + shifty, color);
	Timer0_Wait(100);
}

void oled_drawRipple(int rippleNumber, int rippleStep, oled_color_t color) {
	int i;
	for (i = 0; i < rippleNumber; i++) {
		oled_circle(48, 32, rippleStep * i, color);
		Timer0_Wait(50);
	}
}

void startupAnimation() {
	oled_drawStar(13, 7, OLED_COLOR_WHITE);
	oled_drawStar(13, 7, OLED_COLOR_BLACK);
	oled_drawStar(33, 7, OLED_COLOR_WHITE);
	oled_drawStar(33, 7, OLED_COLOR_BLACK);
	oled_drawRipple(5, 5, OLED_COLOR_WHITE);
	oled_drawRipple(5, 5, OLED_COLOR_BLACK);
	oled_drawRipple(7, 5, OLED_COLOR_WHITE);
	oled_drawRipple(5, 5, OLED_COLOR_BLACK);
	oled_drawStar(23, 5, OLED_COLOR_WHITE);
	oled_putString(32, 32, "STAR-T", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	Timer0_Wait(1500);
	oled_clearScreen(OLED_COLOR_BLACK);
}

int main(void) {
	init();
	startupAnimation();
	loop();
	return 0;
}

void TIMER3_IRQHandler(void) {
	TIM_ClearIntPending(LPC_TIM3, 0);
	updateMode();
}

void EINT0_IRQHandler(void) {
	if ((LPC_SC ->EXTINT >> 0) & 0x1) {
		LPC_SC ->EXTINT |= (1 << 0);

		forceBasicReadFlag = TRUE;
		forceBasicPrintFlag = TRUE;
		readPrintFlag = TRUE;
	}
}

void EINT3_IRQHandler(void) {
	if ((LPC_GPIOINT ->IO2IntStatF >> 5) & 0x1) {
		LPC_GPIOINT ->IO2IntClr = 1 << 5;
		light_clearIrqStatus();

		readLight();

		if (light < FLARE_INTENSITY) {
			light_setLoThreshold(0);
		} else {
			light_setLoThreshold(FLARE_INTENSITY - 1);
			TIM_ResetCounter(LPC_TIM3 );
		}

		updateMode();
	}

	if ((LPC_GPIOINT ->IO0IntStatR >> 2) & 0x1) {
		LPC_GPIOINT ->IO0IntClr = 1 << 2;
		tempRiseTime = usTicks;
	}

	if ((LPC_GPIOINT ->IO0IntStatF >> 2) & 0x1) {
		LPC_GPIOINT ->IO0IntClr = 1 << 2;
		tempDeltaTime = getElapsedTimeUs(tempRiseTime);
		temp = (2 * tempDeltaTime) - 2731;
	}
}

void UART3_IRQHandler(void) {
	uint8_t settings[32];
	static uint8_t type = 0;
	static uint8_t parameter = 0;
	static uint8_t value = 0;

	//RDA
	if ((LPC_UART3 ->IIR & 0x0E) == 0x04) {
		uint8_t data[4];
		UART_Receive(LPC_UART3, &data, 4, BLOCKING);

		if ((data[0] == 'p') && (data[2] == 'v')) {
			switch (data[1]) {
			case UART_SAMPLING_TIME:
				SAMPLING_TIME = data[3] * 100;
				break;
			case UART_FLARE_INTENSITY:
				FLARE_INTENSITY = data[3] * 100;
				light_setHiThreshold(FLARE_INTENSITY);
				break;
			case UART_TIME_UNIT:
				TIME_UNIT = data[3] * 10;
				timer_init(LPC_TIM3, TIME_UNIT);
				TIM_ResetCounter(LPC_TIM3 );
				break;
			case UART_GET_SETTINGS:
				sprintf(settings, "T%d_I%d_U%d\r\n", SAMPLING_TIME / 100,
						FLARE_INTENSITY / 100, TIME_UNIT / 10);
				UART_Send(LPC_UART3, &settings, strlen(settings), BLOCKING);
				break;
			}
		}
	}

	//CTI
	if ((LPC_UART3 ->IIR & 0x0E) == 0x0C) {
		LPC_UART3 ->FCR |= (1 << 1);
	}
}

void SysTick_Handler(void) {
	usTicks++;
}

void check_failed(uint8_t *file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1) {
	}
}


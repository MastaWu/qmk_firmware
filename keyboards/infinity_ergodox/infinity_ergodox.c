#include "infinity_ergodox.h"
#include "ch.h"
#include "hal.h"
#include "serial_link/system/serial_link.h"
#include "lcd_backlight.h"

void init_serial_link_hal(void) {
    PORTA->PCR[1] = PORTx_PCRn_PE | PORTx_PCRn_PS | PORTx_PCRn_PFE | PORTx_PCRn_MUX(2);
    PORTA->PCR[2] = PORTx_PCRn_DSE | PORTx_PCRn_SRE | PORTx_PCRn_MUX(2);
    PORTE->PCR[0] = PORTx_PCRn_PE | PORTx_PCRn_PS | PORTx_PCRn_PFE | PORTx_PCRn_MUX(3);
    PORTE->PCR[1] = PORTx_PCRn_DSE | PORTx_PCRn_SRE | PORTx_PCRn_MUX(3);
}

#define RED_PIN 1
#define GREEN_PIN 2
#define BLUE_PIN 3
#define CHANNEL_RED FTM0->CHANNEL[0]
#define CHANNEL_GREEN FTM0->CHANNEL[1]
#define CHANNEL_BLUE FTM0->CHANNEL[2]

#define RGB_PORT PORTC
#define RGB_PORT_GPIO GPIOC

// Base FTM clock selection (72 MHz system clock)
// @ 0xFFFF period, 72 MHz / (0xFFFF * 2) = Actual period
// Higher pre-scalar will use the most power (also look the best)
// Pre-scalar calculations
// 0 -      72 MHz -> 549 Hz
// 1 -      36 MHz -> 275 Hz
// 2 -      18 MHz -> 137 Hz
// 3 -       9 MHz ->  69 Hz (Slightly visible flicker)
// 4 -   4 500 kHz ->  34 Hz (Visible flickering)
// 5 -   2 250 kHz ->  17 Hz
// 6 -   1 125 kHz ->   9 Hz
// 7 - 562 500  Hz ->   4 Hz
// Using a higher pre-scalar without flicker is possible but FTM0_MOD will need to be reduced
// Which will reduce the brightness range
#define PRESCALAR_DEFINE 0

void lcd_backlight_hal_init(void) {
	// Setup Backlight
    SIM->SCGC6 |= SIM_SCGC6_FTM0;
    FTM0->CNT = 0; // Reset counter

	// PWM Period
	// 16-bit maximum
	FTM0->MOD = 0xFFFF;

	// Set FTM to PWM output - Edge Aligned, Low-true pulses
#define CNSC_MODE FTM_SC_CPWMS | FTM_SC_PS(4) | FTM_SC_CLKS(0)
	CHANNEL_RED.CnSC = CNSC_MODE;
	CHANNEL_GREEN.CnSC = CNSC_MODE;
	CHANNEL_BLUE.CnSC = CNSC_MODE;

	// System clock, /w prescalar setting
	FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_PS(PRESCALAR_DEFINE);

	CHANNEL_RED.CnV = 0;
	CHANNEL_GREEN.CnV = 0;
	CHANNEL_BLUE.CnV = 0;

	RGB_PORT_GPIO->PDDR |= (1 << RED_PIN);
	RGB_PORT_GPIO->PDDR |= (1 << GREEN_PIN);
	RGB_PORT_GPIO->PDDR |= (1 << BLUE_PIN);

#define RGB_MODE PORTx_PCRn_SRE | PORTx_PCRn_DSE | PORTx_PCRn_MUX(4)
    RGB_PORT->PCR[RED_PIN] = RGB_MODE;
    RGB_PORT->PCR[GREEN_PIN] = RGB_MODE;
    RGB_PORT->PCR[BLUE_PIN] = RGB_MODE;
}

void lcd_backlight_hal_color(uint16_t r, uint16_t g, uint16_t b) {
	CHANNEL_RED.CnV = r;
	CHANNEL_GREEN.CnV = g;
	CHANNEL_BLUE.CnV = b;
}

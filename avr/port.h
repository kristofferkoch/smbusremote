#define LED_N		0
#define LED_DDR		DDRC
#define LED_PORT	PORTC
#define LED_PIN		PINC
#define	LED_ON		LED_PORT &= ~(1<<LED_N)
#define LED_OFF		LED_PORT |= (1<<LED_N)
#define LED_TOOGLE	LED_PIN |= (1<<LED_N)

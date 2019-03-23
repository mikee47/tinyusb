/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, hathach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#ifdef BOARD_LPCXPRESSO11U68

#include "chip.h"
#include "../board.h"

#define LED_PORT      2
#define LED_PIN       17
#define LED_STATE_ON  0

static const struct {
  uint8_t port;
  uint8_t pin;
} buttons[] = { { 0, 1 } };

enum {
  BOARD_BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0])
};

/* System oscillator rate and RTC oscillator rate */
const uint32_t OscRateIn = 12000000;
const uint32_t RTCOscRateIn = 32768;

/* Pin muxing table, only items that need changing from their default pin
   state are in this table. Not every pin is mapped. */
static const PINMUX_GRP_T pinmuxing[] =
{
  {0, 3,  (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)}, // USB VBUS
  {0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)}, // UART0 RX
  {0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)}, // UART0 TX
  {2, 0,  (IOCON_FUNC1 | IOCON_MODE_INACT)}, // XTALIN
  {2, 1,  (IOCON_FUNC1 | IOCON_MODE_INACT)}, // XTALOUT
};

// Invoked by startup code
void SystemInit(void)
{
  /* Enable IOCON clock */
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
  Chip_IOCON_SetPinMuxing(LPC_IOCON, pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));
  Chip_SetupXtalClocking();
}

void board_init(void)
{
  SystemCoreClockUpdate();

#if CFG_TUSB_OS == OPT_OS_NONE
  // 1ms tick timer
  SysTick_Config(SystemCoreClock / 1000);
#elif CFG_TUSB_OS == OPT_OS_FREERTOS
  // If freeRTOS is used, IRQ priority is limit by max syscall ( smaller is higher )
  NVIC_SetPriority(USB0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
#endif

  Chip_GPIO_Init(LPC_GPIO);

  //------------- LED -------------//
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED_PORT, LED_PIN);

  //------------- BUTTON -------------//
  //for(uint8_t i=0; i<BOARD_BUTTON_COUNT; i++) GPIOSetDir(buttons[i].port, buttons[i].pin, 0);

  //------------- UART -------------//
  //UARTInit(CFG_UART_BAUDRATE);

  // USB
  Chip_USB_Init(); // Setup PLL clock, and power
}

/*------------------------------------------------------------------*/
/* TUSB HAL MILLISECOND
 *------------------------------------------------------------------*/
#if CFG_TUSB_OS == OPT_OS_NONE

volatile uint32_t system_ticks = 0;

void SysTick_Handler (void)
{
  system_ticks++;
}

uint32_t tusb_hal_millis(void)
{
  return board_tick2ms(system_ticks);
}

uint32_t board_noos_millis(void)
{
  return system_ticks;
}

#endif

//--------------------------------------------------------------------+
// LEDS
//--------------------------------------------------------------------+
void board_led_control(bool state)
{
  Chip_GPIO_SetPinState(LPC_GPIO, LED_PORT, LED_PIN, state ? LED_STATE_ON : (1-LED_STATE_ON));
}

//--------------------------------------------------------------------+
// Buttons
//--------------------------------------------------------------------+
uint32_t board_buttons(void)
{
//  for(uint8_t i=0; i<BOARD_BUTTON_COUNT; i++) GPIOGetPinValue(buttons[i].port, buttons[i].pin);
//  return GPIOGetPinValue(buttons[0].port, buttons[0].pin) ? 0 : 1; // button is active low
  return 0;
}

//--------------------------------------------------------------------+
// UART
//--------------------------------------------------------------------+
int board_uart_read(uint8_t* buf, int len)
{
//  *buffer = get_key(); TODO cannot find available code for uart getchar
  (void) buf;
  (void) len;
  return 0;
}

int board_uart_write(void const * buf, int len)
{
  //UARTSend(&c, 1);
  (void) buf;
  (void) len;
  return 0;
}

#endif

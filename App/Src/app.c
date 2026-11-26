#include "app.h"
#include "main.h"          /* HAL, LD1 pin defines */
#include "usbd_cdc_if.h"   /* usb_read, usb_printf, usb_tx_dropped, usb_tx_free */

static uint32_t tick_n    = 0;
static uint32_t tick_last = 0;

void app_init(void)
{
  tick_last = HAL_GetTick();
}

void app_loop(void)
{
  uint8_t rx[64];

  /* echo: whatever the PC sends comes back */
  uint32_t len = usb_read(rx, sizeof(rx));
  if (len > 0)
  {
    if (rx[0] == 'b')       /* burst test: 100 lines, about 4 KB */
    {
      for (uint32_t i = 0; i < 100; i++)
        usb_printf("burst %03lu  0123456789abcdef\r\n", i);
    }
    else
    {
      usb_printf("echo: %.*s\r\n", (int)len, rx);
    }
  }

  /* every 500 ms: blink + tick */
  if (HAL_GetTick() - tick_last >= 500)
  {
    tick_last = HAL_GetTick();
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    usb_printf("tick %lu  ms=%lu  pi=%.3f  drop=%lu  free=%lu\r\n",
               tick_n, HAL_GetTick(), 3.14159f, usb_tx_dropped(), usb_tx_free());
    tick_n++;
  }
}
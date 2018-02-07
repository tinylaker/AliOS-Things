
#include "espressif/esp_common.h"
#include "gpio.h"

#define KEY_GPIO_PIN GPIO_Pin_14
#define KEY_GPIO_MODE GPIO_Mode_Input
#define KEY_GPIO_PULLUP GPIO_PullUp_DIS
#define KEY_GPIO_INTRTYPE GPIO_PIN_INTR_NEGEDGE

static void key_gpio_enable_isr(void)
{
    _xt_isr_unmask(1 << ETS_GPIO_INUM);
}

static void key_gpio_isr(void *arg)
{
    uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    ets_printf("KEY is %x\n", gpio_status);
}

void key_gpio_init(void)
{
    GPIO_ConfigTypeDef key_gpio_cnf;

    gpio_intr_handler_register(key_gpio_isr, NULL);

    key_gpio_cnf.GPIO_Pin = KEY_GPIO_PIN;
    key_gpio_cnf.GPIO_Mode = KEY_GPIO_MODE;
    key_gpio_cnf.GPIO_Pullup = KEY_GPIO_PULLUP;
    key_gpio_cnf.GPIO_IntrType = KEY_GPIO_INTRTYPE;
    gpio_config(&key_gpio_cnf);

    key_gpio_enable_isr();
}

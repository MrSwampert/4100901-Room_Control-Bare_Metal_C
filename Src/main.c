#include "gpio.h"
#include "systick.h"
#include "nvic.h"
#include "uart.h"
#include "tim.h"
#include "room_control.h"

void heartbeat_led_toggle(void)
{
    static uint32_t last_tick = 0;
    if (systick_get_tick() - last_tick >= 1000) { // Cambia cada 500 ms
        gpio_toggle_pin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
        last_tick = systick_get_tick();
    }
}


int main(void)
{
    // Inicialización de SysTick
    systick_init_1ms();

    // LED Heartbeat (PA5 - LD2)
    gpio_setup_pin(GPIOA, 5, GPIO_MODE_OUTPUT, 0);

    // LED Externo ON/OFF (PA7)
    gpio_setup_pin(GPIOA, 7, GPIO_MODE_OUTPUT, 0);

    // Botón B1 (PC13)
    gpio_setup_pin(GPIOC, 13, GPIO_MODE_INPUT, 0);
    nvic_exti_pc13_button_enable();

    // USART2 (PA2-TX, PA3-RX)
    uart2_init(115200);
    nvic_usart2_irq_enable();

    // TIM3 Canal 1 para PWM (PA6)
    tim3_ch1_pwm_init(1000); // 1000 Hz
    tim3_ch1_pwm_set_duty_cycle(70); // 70% duty cycle inicial

    // Inicialización de la Lógica de la Aplicación
    room_control_app_init();

    uart2_send_string("\r\nSistema Inicializado. Esperando eventos...\r\n");
    
    while (1) {
        heartbeat_led_toggle();
        room_control_process();
        
        // Pequeño delay para evitar consumo excesivo de CPU
        systick_delay_ms(10);
    }
}

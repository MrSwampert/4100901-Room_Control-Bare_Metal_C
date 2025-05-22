#include "room_control.h"
#include "gpio.h"
#include "systick.h"
#include "uart.h"
#include "tim.h"

// Variables de estado (static para mantener privacidad)
static volatile uint32_t led_onoff_start_time = 0;
static volatile uint8_t led_onoff_active = 0;
static volatile uint32_t last_button_press_time = 0;

void room_control_app_init(void)
{
    // Asegurar que los LEDs estén apagados al inicio
    gpio_write_pin(GPIOA, 7, GPIO_PIN_RESET); // LED ON/OFF apagado
    tim3_ch1_pwm_set_duty_cycle(70); // PWM al 70%
}

void room_control_on_button_press(void)
{
    uint32_t current_time = systick_get_tick();
    
    // Anti-rebote: ignorar pulsaciones muy cercanas (200ms)
    if ((current_time - last_button_press_time) < 200) {
        return;
    }
    
    last_button_press_time = current_time;
    
    // Encender LED ON/OFF
    gpio_write_pin(GPIOA, 7, GPIO_PIN_SET);
    led_onoff_start_time = current_time;
    led_onoff_active = 1;
    
    uart2_send_string("Boton B1: Presionado. LED encendido por 3 segundos.\r\n");
}

void room_control_on_uart_receive(char received_char)
{
    // Eco del carácter recibido (ya se hace en USART2_IRQHandler)
    
    // Procesar comandos
    switch(received_char) {
        case 'h':
        case 'H':
            tim3_ch1_pwm_set_duty_cycle(100); // 100% duty cycle
            uart2_send_string("PWM al 100%\r\n");
            break;
            
        case 'l':
        case 'L':
            tim3_ch1_pwm_set_duty_cycle(0); // 0% duty cycle (apagado)
            uart2_send_string("PWM al 0%\r\n");
            break;
            
        case 't':
            gpio_toggle_pin(GPIOA, 7); // Toggle LED ON/OFF
            uart2_send_string("Toggle LED ON/OFF\r\n");
            break;
            
        default:
            // Opcional: enviar mensaje de comando no reconocido
            break;
    }
}

void room_control_process(void)
{
    // Esta función debe ser llamada periódicamente (ej. desde el main loop)
    // para manejar tareas temporizadas
    
    if (led_onoff_active && (systick_get_tick() - led_onoff_start_time >= 3000)) {
        gpio_write_pin(GPIOA, 7, GPIO_PIN_RESET);
        led_onoff_active = 0;
        uart2_send_string("LED apagado después de 3 segundos.\r\n");
    }
}
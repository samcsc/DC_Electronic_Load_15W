#include "Interrupt_Callback.h"
#include "main.h"
#include "cmsis_os.h"


extern uint8_t pb_a_pressed;
extern uint8_t pb_inc_pressed;
extern uint8_t pb_dec_pressed;

extern osThreadId_t push_buttom_task_handle;

// ------------------------------------------------------------------------- //

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	switch (GPIO_Pin) {
	case BT_A_Pin:
		pb_a_pressed = 1;
		break;
	case BT_INC_Pin:
		pb_inc_pressed = 1;
		break;
	case BT_DEC_Pin:
		pb_dec_pressed = 1;
		break;
	default:
		break;
	}
	xTaskResumeFromISR(push_buttom_task_handle);
}

// ------------------------------------------------------------------------- //

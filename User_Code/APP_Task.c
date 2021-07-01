#include "APP_Task.h"
#include "FreeRTOS.h"
#include "BSP.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "main.h"
#include "inttypes.h"
#include "math.h"


struct Device {
	// Device reading
	float temperature;
	float voltage_measured;
	float current_measured;

	// Setting
	uint8_t tcurrent_index;
	float current_updated;
	uint8_t device_start;

	// Error
	uint8_t error;
	uint8_t error_overtemp;
} device;

float current_list[] = { 0, 0.1, 0.3, 0.5, 0.75, 1, 1.5, 2, 2.3};

uint8_t pb_a_pressed = 0;
uint8_t pb_inc_pressed = 0;
uint8_t pb_dec_pressed = 0;

/* Definitions for mutex */
osMutexId_t mutex_device_status_handle;
const osMutexAttr_t mutex_device_status_attr = { .name = "Mutex Device Status" };

/* Task handle and task function prototype */
osThreadId_t temperature_task_handle;
void Temperature_Task(void *argument);
const osThreadAttr_t temperature_task_attr = { .name = "Temperature Task",
		.priority = (osPriority_t) osPriorityHigh, .stack_size = 400 * 4 };

osThreadId_t led_task_handle;
void LED_Task(void *argument);
const osThreadAttr_t led_task_attr = { .name = "LED Task", .priority =
		(osPriority_t) osPriorityLow, .stack_size = 150 * 4 };

osThreadId_t push_buttom_task_handle;
void Push_Button_Task(void *argument);
const osThreadAttr_t pb_task_attr = { .name = "Push Buttom Task", .priority =
		(osPriority_t) osPriorityLow, .stack_size = 150 * 4 };

osThreadId_t current_regulation_task_handle;
void Current_Regulation_Task(void *argument);
const osThreadAttr_t current_reg_task_attr = {
		.name = "Current Regulation Task", .priority =
				(osPriority_t) osPriorityAboveNormal, .stack_size = 250 * 4 };

osThreadId_t data_strem_task_handle;
void Data_Stream_Task(void *argument);
const osThreadAttr_t data_stream_task_attr = { .name = "Data Stream Task",
		.priority = (osPriority_t) osPriorityLow, .stack_size = 500 * 4 };

// Debug task
osThreadId_t debug_task_handle;
void Debug_Task(void *argument);
const osThreadAttr_t debug_task_attr = { .name = "Debug Task", .priority =
		(osPriority_t) osPriorityLow, .stack_size = 300 * 4 };

/* These functions run before osKernelStart */
void mutex_creation() {
	mutex_device_status_handle = osMutexNew(&mutex_device_status_attr);
}
void task_creation() {
	temperature_task_handle = osThreadNew(Temperature_Task, NULL,
			&temperature_task_attr);
	led_task_handle = osThreadNew(LED_Task, NULL, &led_task_attr);
	push_buttom_task_handle = osThreadNew(Push_Button_Task, NULL,
			&pb_task_attr);
	current_regulation_task_handle = osThreadNew(Current_Regulation_Task, NULL,
			&current_reg_task_attr);
	data_strem_task_handle = osThreadNew(Data_Stream_Task, NULL,
			&data_stream_task_attr);
	debug_task_handle = osThreadNew(Debug_Task, NULL, &debug_task_attr);
}

/*
 * The main function of Temperature_Task is to get the temperature reading
 * from the NTC thermistor and issue an over temperature warning if the
 * temperature is over MAX_TEMP_HYSTERESIS_H.
 */
void Temperature_Task(void *argument) {
	float temperature = 0;
	const float MAX_TEMP_HYSTERESIS_H = 75;
	const float MAX_TEMP_HYSTERESIS_L = 60;
	while (TRUE) {
		temperature = get_temperature();
		device.temperature = temperature;
		// TODO: What if measurement isn't correct, like ADC fail, sensor broken, etc.
		if (isnan(temperature) || temperature < -50 || temperature > 150) {
			device.error = 1;
		}

		// Over temperature protection
		if (device.temperature > MAX_TEMP_HYSTERESIS_H) {
			device.error_overtemp = 1;
		}
		if (device.error_overtemp
				&& device.temperature < MAX_TEMP_HYSTERESIS_L) {
			device.error_overtemp = 0;
		}

		osDelay(1000);
	}
}

void LED_Task(void *argument) {
	while (TRUE) {
		if (device.error_overtemp || device.error) {
			led_all_on();
			osDelay(250);
			led_all_off();
			osDelay(250);
		} else {
			if (device.device_start)
				led_run_on();
			else
				led_run_off();

			led_current_indication(device.tcurrent_index);

			osDelay(50);
		}

	}
}

/*
 *
 */
void Push_Button_Task(void *argument) {
	while (TRUE) {

		// Wait for button ISR to unblock it
		osThreadSuspend(osThreadGetId());

		if (pb_a_pressed) {
			pb_a_pressed = 0;
			device.current_updated = 1;
		} else if (pb_inc_pressed) {
			pb_inc_pressed = 0;
			if (device.tcurrent_index < 8)
				device.tcurrent_index++;
			else
				device.tcurrent_index = 0;
		} else if (pb_dec_pressed) {
			pb_dec_pressed = 0;
			if (device.tcurrent_index > 0)
				device.tcurrent_index--;
		}
	}
}

void Current_Regulation_Task(void *argument) {

	float target_current = 0;
	float measured_voltage = 0;
	float measured_current = 0;
	float err = 0;
	float errI = 0;
	float errP = 0;

	// Make sure the output is turned off when first start.
	set_dac_voltage(-1);

	while (TRUE) {

		uint32_t entry_tick = osKernelGetTickCount();
		led_pc_on();

		// Take measurements
		measured_voltage = get_voltage();
		measured_current = get_current();
		device.voltage_measured = measured_voltage;
		device.current_measured = measured_current;

		// Turn the DAC off if there is an error.
		if (device.error || device.error_overtemp) {
			set_dac_voltage(-1);
		}
		// If no error, go ahead to set the current
		else {

			if (device.current_updated) {
				device.current_updated = 0;
				if (target_current == current_list[device.tcurrent_index]) {
					target_current = 0;
					device.device_start = 0;
				} else {
					target_current = current_list[device.tcurrent_index];
					device.device_start = 1;
				}
				errI = 0;
				set_dac_voltage(target_current);
			} else {

				err = target_current - measured_current;
				errP = err * 0.2;
				errI += err * 0.15;
				if (errI > target_current * 0.15) {
					errI = target_current * 0.15;
				} else if (errI < target_current * -0.15) {
					errI = target_current * -0.15;
				}
				err = errP + errI;
				set_dac_voltage(target_current + err);

			}

		}

		led_pc_off();
		osDelayUntil(entry_tick + 200);
	}
}

void Data_Stream_Task(void *argument) {
	struct Device d;
	static char txbuffer[80];
	uint32_t txbuffer_size = 0;
	// Opening message
	txbuffer_size = sprintf(txbuffer,
			"Time (s), Voltage (V), Load Current (I), Temperature (degC)\n");
	uart_transmit_dma((uint8_t*) txbuffer, txbuffer_size);
	osDelay(5);
	while (TRUE) {
		osMutexAcquire(mutex_device_status_handle, osWaitForever);
		d = device;
		osMutexRelease(mutex_device_status_handle);

		txbuffer_size = sprintf(txbuffer, "%0.3f, %0.3f, %0.1f\n",
				d.voltage_measured, d.current_measured, d.temperature);
		uart_transmit_dma((uint8_t*) txbuffer, txbuffer_size);

		osDelay(1000);

	}
}

/* This section is for Debug Task and its helper functions */
static void debug_message(const char *tname, uint32_t watermark) {
	char txbuffer[80];
	uint32_t txbuffer_size = 0;
	txbuffer_size = sprintf(txbuffer, "%s stack remains %u bytes\n", tname,
			(int) watermark);
	for (int i = 0; i < txbuffer_size; i++) {
		ITM_SendChar((uint8_t) txbuffer[i]);
	}
}

void Debug_Task(void *argument) {
	while (TRUE) {
		uint32_t entry_tick = osKernelGetTickCount();

		debug_message(debug_task_attr.name,
				osThreadGetStackSpace(debug_task_handle));
		debug_message(temperature_task_attr.name,
				osThreadGetStackSpace(temperature_task_handle));
		debug_message(led_task_attr.name,
				osThreadGetStackSpace(led_task_handle));
		debug_message(pb_task_attr.name,
				osThreadGetStackSpace(push_buttom_task_handle));
		debug_message(data_stream_task_attr.name,
				osThreadGetStackSpace(data_strem_task_handle));
		debug_message(current_reg_task_attr.name,
				osThreadGetStackSpace(current_regulation_task_handle));
		ITM_SendChar('\n');

		osDelayUntil(entry_tick + 5000);
	}
}

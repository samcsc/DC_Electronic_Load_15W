#include "BSP.h"
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "cmsis_os.h"

extern ADC_HandleTypeDef 	hadc1;
extern UART_HandleTypeDef 	hlpuart1;
extern I2C_HandleTypeDef 	hi2c3;
extern SPI_HandleTypeDef 	hspi2;

// Transmit data using UART DMA.
int uart_transmit_dma(uint8_t *pBuffer, uint16_t buffer_size)
{
	HAL_StatusTypeDef status = HAL_OK;

//	static uint32_t time_last = 0;
//	uint32_t time = HAL_GetTick();
//
//	if (time - time_last > 50) {
//		time_last = time;
//		status = HAL_UART_Transmit_DMA(&hlpuart1, pBuffer, buffer_size);
//	}
	status = HAL_UART_Transmit_DMA(&hlpuart1, pBuffer, buffer_size);
	if (status != HAL_OK) {
		__NOP();
	}

	return status;

}

// Read the temperature from temperature sensor (NTC).
// The return value should be within -20 to 150 degC, any
// values outside this range should be consider error.
// It might be broken sensor or ADC reading failed.
float get_temperature()
{
	float ret = -99; 	/* Default error code */
	uint16_t raw;
	float adc_volt = 0;
	float sensor_res = 0;

	// Some calibration values
	// Equation to calculate temperature from
	// NTC resistance: T = A+B*log(R)+C*log(R)^3
	static const float A = 376.7;
	static const float B = -47.07;
	static const float C = 0.1053;
	static const int RESBOT = 2000;
	static const float TO_VOLT = 3.3 / (4096 - 1);

	// Get ADC raw reading
	if (HAL_ADC_Start(&hadc1) != HAL_OK)
		return -99;
	if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK)
		return -99;
	raw = HAL_ADC_GetValue(&hadc1);

	// Convert the raw reading to temperature (degC).
	adc_volt = raw * TO_VOLT;
	sensor_res = RESBOT * (3.3 / adc_volt - 1);
	ret = A + B * log(sensor_res) + C * pow(log(sensor_res), 3);

	return ret;
}

// Set the DAC (MCP4821) output voltage.
int set_dac_voltage(float voltage)
{
	uint8_t data[2];
	uint16_t setDacVoltageCode = 0;

	// Turn the DAC off if the input value is negative or 0.
	if (voltage <= 0) {
		data[0] = 0;
		data[1] = 0;
	} else if (voltage < 2.00) {
		setDacVoltageCode = voltage * 4096 / 2.048;
		data[0] = 0b00110000 | (setDacVoltageCode >> 8);
		data[1] = setDacVoltageCode;
	} else if (voltage < 2.7) {
		setDacVoltageCode = voltage * 4096 / 4.096;
		data[0] = 0b00010000 | (setDacVoltageCode >> 8);
		data[1] = setDacVoltageCode;
	} else {
		voltage = 2.7;
		setDacVoltageCode = voltage * 4096 / 4.096;
		data[0] = 0b00010000 | (setDacVoltageCode >> 8);
		data[1] = setDacVoltageCode;
	}

	HAL_GPIO_WritePin(CS_DAC_GPIO_Port, CS_DAC_Pin, GPIO_PIN_RESET);
	osDelay(1);
	if (HAL_SPI_Transmit(&hspi2, data, 2, 1000) != HAL_OK) {
		return -1;
	}
	HAL_GPIO_WritePin(CS_DAC_GPIO_Port, CS_DAC_Pin, GPIO_PIN_SET);

	return 0;
}

// MCP3426 related define
#define MCP3426_ADDR (0x68 << 1)
#define VOLTAGE_GAIN 0.0006253209
#define VOLTAGE_OFFSET 0.0004902258
#define CURRENT_GAIN 0.0000725723
#define CURRENT_OFFSET 0.0001016406

// Get the voltage measurement from MCP3426.
float get_voltage()
{
	uint8_t txData = 0b10101000;
	uint8_t rxData[2] = { 0 };
	int16_t result = 0;

	if (HAL_I2C_Master_Transmit(&hi2c3, MCP3426_ADDR, &txData, 1, 1000) != HAL_OK)
		return -1;

	osDelay(67); // Take some time for the conversion to complete

	if (HAL_I2C_Master_Receive(&hi2c3, MCP3426_ADDR, rxData, 2, 1000) != HAL_OK)
		return -1;

	result = rxData[0] << 8 | rxData[1];

	return result * VOLTAGE_GAIN + VOLTAGE_OFFSET;
}

// Get the current measurement from MCP3426.
float get_current()
{
	uint8_t txData = 0b10001001;
	uint8_t rxData[2];
	int16_t result = 0;

	if (HAL_I2C_Master_Transmit(&hi2c3, MCP3426_ADDR, &txData, 1, 1000) != HAL_OK) {
		return -1;
	}

	osDelay(67); // Take some time for the conversion to complete

	if (HAL_I2C_Master_Receive(&hi2c3, MCP3426_ADDR, rxData, 2, 1000) != HAL_OK) {
		return -1;
	}

	result = rxData[0] << 8 | rxData[1];

	return result * CURRENT_GAIN + CURRENT_OFFSET;
}


//
void led_all_on()
{
	HAL_GPIO_WritePin(LED_PC_GPIO_Port, LED_PC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_100_GPIO_Port, LED_100_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_500_GPIO_Port, LED_500_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_1000_GPIO_Port, LED_1000_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_2000_GPIO_Port, LED_2000_Pin, GPIO_PIN_SET);
}

//
void led_all_off()
{
	HAL_GPIO_WritePin(LED_PC_GPIO_Port, LED_PC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_100_GPIO_Port, LED_100_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_500_GPIO_Port, LED_500_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_1000_GPIO_Port, LED_1000_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_2000_GPIO_Port, LED_2000_Pin, GPIO_PIN_RESET);
}

//
void led_pc_on()
{
	HAL_GPIO_WritePin(LED_PC_GPIO_Port, LED_PC_Pin, GPIO_PIN_SET);
}

//
void led_pc_off()
{
	HAL_GPIO_WritePin(LED_PC_GPIO_Port, LED_PC_Pin, GPIO_PIN_RESET);
}

void led_run_on()
{
	HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, GPIO_PIN_SET);
}
void led_run_off()
{
	HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, GPIO_PIN_RESET);
}

//
void led_current_indication(uint8_t index)
{
	HAL_GPIO_WritePin(LED_100_GPIO_Port, LED_100_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_500_GPIO_Port, LED_500_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_1000_GPIO_Port, LED_1000_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_2000_GPIO_Port, LED_2000_Pin, GPIO_PIN_RESET);
	switch (index) {
		case 0:
			/* Already off */
			break;
		case 1:
			HAL_GPIO_WritePin(LED_100_GPIO_Port, LED_100_Pin, GPIO_PIN_SET);
			break;
		case 2:
			HAL_GPIO_WritePin(LED_100_GPIO_Port, LED_100_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_500_GPIO_Port, LED_500_Pin, GPIO_PIN_SET);
			break;
		case 3:
			HAL_GPIO_WritePin(LED_500_GPIO_Port, LED_500_Pin, GPIO_PIN_SET);
			break;
		case 4:
			HAL_GPIO_WritePin(LED_500_GPIO_Port, LED_500_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_1000_GPIO_Port, LED_1000_Pin, GPIO_PIN_SET);
			break;
		case 5:
			HAL_GPIO_WritePin(LED_1000_GPIO_Port, LED_1000_Pin, GPIO_PIN_SET);
			break;
		case 6:
			HAL_GPIO_WritePin(LED_1000_GPIO_Port, LED_1000_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_2000_GPIO_Port, LED_2000_Pin, GPIO_PIN_SET);
			break;
		case 7:
			HAL_GPIO_WritePin(LED_2000_GPIO_Port, LED_2000_Pin, GPIO_PIN_SET);
			break;
		case 8:
			HAL_GPIO_WritePin(LED_100_GPIO_Port, LED_100_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_500_GPIO_Port, LED_500_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_1000_GPIO_Port, LED_1000_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_2000_GPIO_Port, LED_2000_Pin, GPIO_PIN_SET);
		default:
			break;
	}

}


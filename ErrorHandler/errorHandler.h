/*
 * errorHandler.h
 *
 *  Created on: Sep 21, 2019
 *  Author: karsh
 */

#ifndef MAIN_ERRORHANDLER_ERRORHANDLER_H_
#define MAIN_ERRORHANDLER_ERRORHANDLER_H_


#include "stddef.h"
#include "stdlib.h"
#include "string"

using namespace std;




/*****************************
	enumeration: emu_err_t
*******************************/
/**
 * @brief
 * @details
 *
 *
 * @see
 *
 */
typedef enum{
	EMU_SUCCESS=0,

	//------------------------------
	EMU_ERROR_MAJOR=-30,
	EMU_ERR_OUT_OF_MEMORY,
	EMU_ERR_INVALID_ARG,
	EMU_ERR_NULL,
	EMU_ERR_ILLEGAL_ACCESS,
	EMU_ERR_INIT_FAILED,
	EMU_SENSOR_COMM_INTERFACE_NOT_FOUND,
	EMU_ERR_TASK_TIMER,
	//------------------------------
	EMU_ERROR_MINOR=-60,
	EMU_SENSOR_COMM_INTERFACE_NOT_SUPPORTED,

	//------------------------------
	EMU_WARN_MAJOR=-90,


	//------------------------------
	EMU_WARN_MINOR=-120,



	//------Yet to add -------------
	EMU_FAILURE=-1,
	EMU_NO_MATCHING=-2,
	EMU_WARN_NO_CALLBACK=-3,

	EMU_ERR_ACCESS_DENIED=-5,




	EMU_ERR_EVT_HANDLER_REGISTRATION=-10,
	EMU_ERR_EVT_HANDLER_DEREGISTRATION=-11,
	EMU_ERR_EVT_TASK_CREATION=-12,
	EMU_ERR_EVT_LOOP_CREATION=-13,


	EMU_ERR_NULL_INSTANCE=-16,
	EMU_ERR_TEST_FAILED=-17,
	EMU_ERR_MQTT_CLIENT_PUB=-18,
	EMU_ERR_MQTT_CLIENT_SUB=-19,
	EMU_ERR_GPS_SETUP = -21,
	EMU_ERR_BMI160_INIT = -22,
	EMU_ERR_GENERIC=-23,
	EMU_ERR_STD_FAILURE = -24,
	EMU_ERR_NAN = -25,

	//Communication periperal
	EMU_ERR_WIFI_DISCONNECTION=-14,
	EMU_ERR_WIFI_CONNECTION=-15,
	EMU_PASS=-27
	//EMU_COMM_PERIPHERAL_NOT_INITIALIZED=27
}emu_err_t;


typedef enum{
	TURN_LED_OFF=0,
	TURN_LED_ON,
	TOGGLE_LED_1HZ,
	TOGGLE_LED_2HZ,
	TOGGLE_LED_4HZ,
	TURN_LED_RED,
	TURN_LED_GREEN,
	TURN_LED_BLUE,
	TERMINATE,
	SPIN_FOREVER,
	SOFTWARE_RESET,
	SEND_ERROR_MSG_MQTT,
	SEND_ERROR_MSG_TCP,
	SEND_ERROR_MSG_WS,
	SEND_ERROR_MSG_BLE,
	SEND_ERROR_MSG_UART0,
	SEND_ERROR_MSG_UART1,
	SEND_ERROR_MSG_UART2,
	SEND_ERROR_MSG_I2C0,
	SEND_ERROR_MSG_I2C1,
	SEND_ERROR_MSG_HSSPI,
	SEND_ERROR_MSG_LSSPI,
	SEND_ERROR_MSG_SD_CARD_LOGGER,
	JUST_SHOW_DEBUG_LOG
}error_reporting_action_e;


typedef struct{
	error_reporting_action_e std_exception;
	error_reporting_action_e error_major;
	error_reporting_action_e error_minor;
	error_reporting_action_e warn_major;
	error_reporting_action_e warn_minor;
}error_policy_t;

typedef enum{
	STD_EXCEPTION=0,
	ERROR_MAJOR,
	ERROR_MINOR,
	WARN_MAJOR,
	WARN_MINOR,
	NOT_AN_ERROR
}error_category_t;

/*****************************
	Callback typedef
*******************************/
/**
 * @brief
 * @details
 *
 *
 * @see
 *
 */
typedef void (appErrorCallback)(emu_err_t error,error_policy_t errorPolicy,void* arg1,size_t arg1Size);
typedef void (stdExceptionCallback)(exception error,error_policy_t errorPolicy,void* arg1,size_t arg1Size);

extern error_policy_t defaultErrorPolicy;

/***************************************
	class: errorHandler
***************************************/
/**
 * @brief
 * @details
 *
 *
 * @see
 *
 */
class errorHandler{
	public:
		static const char* printError(emu_err_t err);
		static void processError(emu_err_t error, error_policy_t errorPolicy=defaultErrorPolicy,appErrorCallback func=NULL,void* arg1=NULL,size_t arg1Size=0);
		static void processError(exception error, error_policy_t errorPolicy=defaultErrorPolicy,stdExceptionCallback func=NULL,void* arg1=NULL,size_t arg1Size=0);
		static error_category_t getErrorCategory(emu_err_t error);
		static void perform_action(error_reporting_action_e action);
};


#endif /* MAIN_ERRORHANDLER_ERRORHANDLER_H_ */

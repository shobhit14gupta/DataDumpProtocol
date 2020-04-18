/*
 * errorHandler.cpp
 *
 *  Created on: Sep 21, 2019
 *  Author: karsh
 */
#include "errorHandler.h"


error_policy_t defaultErrorPolicy={
		.std_exception=TOGGLE_LED_4HZ,
		.error_major=TOGGLE_LED_4HZ,
		.error_minor=TOGGLE_LED_2HZ,
		.warn_major=TOGGLE_LED_1HZ,
		.warn_minor=JUST_SHOW_DEBUG_LOG
};

/**
 * @brief
 * @details
 *
 * @param[]
 *
 *
 * @return
 *
 * @see
 *
 */
void errorHandler::processError(emu_err_t error, error_policy_t errorPolicy,appErrorCallback cb,void* arg1,size_t arg1Size)  {
	if(cb!=NULL){
		cb(error,errorPolicy,arg1,arg1Size);
	}else{
		switch(errorHandler::getErrorCategory(error)){
		case ERROR_MAJOR:
			errorHandler::perform_action(errorPolicy.error_major);
			break;
		case ERROR_MINOR:
			errorHandler::perform_action(errorPolicy.error_minor);
			break;
		case WARN_MAJOR:
			errorHandler::perform_action(errorPolicy.warn_major);
			break;
		case WARN_MINOR:
			errorHandler::perform_action(errorPolicy.error_minor);
			break;
		default:
			break;
		}
	}

}

void errorHandler::processError(exception error,error_policy_t errorPolicy,stdExceptionCallback cb,void* arg1,size_t arg1Size) {
	if(cb!=NULL){
			cb(error,errorPolicy,arg1,arg1Size);
	}else{
		errorHandler::perform_action(errorPolicy.std_exception);
	}
}


error_category_t errorHandler::getErrorCategory(emu_err_t error)  {
	if(error>=EMU_ERROR_MAJOR && error<EMU_SUCCESS)
		return ERROR_MAJOR;
	else if(error>=EMU_ERROR_MINOR && error<EMU_ERROR_MAJOR)
		return ERROR_MINOR;
	else if(error>=WARN_MAJOR && error<EMU_ERROR_MINOR)
		return WARN_MAJOR;
	else
		return WARN_MINOR;

}




void errorHandler::perform_action(error_reporting_action_e action)  {

}


const char* errorHandler::printError(emu_err_t err) {
	string str="";
	switch(err){
	case EMU_SUCCESS:
		str="#EMU_SUCCESS";
	break;
	case EMU_FAILURE:
		str="#EMU_FAILURE";
	break;
	case EMU_NO_MATCHING:
		str="#EMU_NO_MATCHING";
	break;
	case EMU_WARN_NO_CALLBACK:
		str="#EMU_WARN_NO_CALLBACK";
	break;
	case EMU_ERR_INVALID_ARG:
		str="#EMU_ERR_INVALID_ARG";
	break;
	case EMU_ERR_ACCESS_DENIED:
		str="#EMU_ERR_ACCESS_DENIED";
	break;
	case EMU_ERR_OUT_OF_MEMORY:
		str="#EMU_ERR_OUT_OF_MEMORY";
	break;
	case EMU_ERR_NULL:
		str="#EMU_ERR_NULL";
	break;
	case EMU_ERR_ILLEGAL_ACCESS:
		str="#EMU_ERR_ILLEGAL_ACCESS";
	break;
	case EMU_ERR_EVT_HANDLER_REGISTRATION:
		str="#EMU_ERR_EVT_HANDLER_REGISTRATION";
		break;
	case EMU_ERR_EVT_HANDLER_DEREGISTRATION:
		str="#EMU_ERR_EVT_HANDLER_DEREGISTRATION";
		break;
	case EMU_ERR_EVT_TASK_CREATION:
		str="#EMU_ERR_EVT_TASK_CREATION";
		break;
	case EMU_ERR_GENERIC:
		str="#EMU_ERR_GENERIC";
	break;
	default:
		str="UNKNOWN, FORGOT";
	}
	return str.c_str();
}




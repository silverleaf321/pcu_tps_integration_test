// ********************************** Includes & External **********************************

#include "FEB_Main.h"
#include <stdbool.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;

// ********************************** Variables **********************************
char buf[128];
uint8_t buf_len; //stolen from Main_Setup (SN2)

#define TPS2482_MONITOR_INTERVAL 100
uint32_t lastTPS2482ReadTime = 0;

// ********************************** Functions **********************************

void FEB_Main_Setup(void){
	HAL_ADCEx_InjectedStart(&hadc1); //@lovehate - where does this go
//	FEB_Timer_Init();
	FEB_TPS2482_Setup();
	FEB_CAN_Init(); //FEB_CAN_Init() // The transceiver must be connected otherwise you get sent into an infinite loop
	FEB_CAN_RMS_Setup();
}

void FEB_Main_While(void){
//	FEB_CAN_ICS_Transmit();
//
    uint32_t currentTime = HAL_GetTick();
	FEB_SM_ST_t bms_state = FEB_CAN_BMS_getState();


	if (FEB_Ready_To_Drive() && (bms_state == FEB_SM_ST_DRIVE || bms_state == FEB_SM_ST_DRIVE_REGEN)) {
		FEB_Normalized_updateAcc();
		FEB_CAN_RMS_Process();
	} else {
		FEB_Normalized_setAcc0();
		FEB_CAN_RMS_Disable();
	}
	FEB_HECS_update();

	FEB_CAN_RMS_Torque();
	FEB_Normalized_CAN_sendBrake();

	// TPS Stuff
	if (currentTime - lastTPS2482ReadTime >= TPS2482_MONITOR_INTERVAL) {
	        FEB_TPS2482_sendReadings();

	        // should check for errors here

	        lastTPS2482ReadTime = currentTime;
	    }

	HAL_Delay(10);
}

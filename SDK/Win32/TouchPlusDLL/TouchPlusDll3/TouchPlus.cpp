// TouchPlus.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "TouchPlus.h"
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

#include "eSPAEAWBCtrl.h"
#include "EtronDI_O.h"
#include <stdexcept>
#include "CameraDS.h"
#define TP_CAMERA_VID   "1e4e"
#define TP_CAMERA_PID   "0107"

using namespace std;

namespace TouchFuncs
{
	void * pHandle = NULL;
	CCameraDS                          *ds_camera_;

	int MyTouchFuncs::isCameraPresent(){
		int devCount;
		int didInit = EtronDI_Init(&pHandle);
		int devNumber = EtronDI_GetDeviceNumber(pHandle);

		if (VERBOSE) printf("device count = %d\n", devNumber);
		WCHAR name[100];
		if (!EtronDI_FindDevice(pHandle)) {
			fprintf(stderr, "Device not found!\n");
			return 0;
		}
		//int deviceSelected = EtronDI_SelectDevice(pHandle, 0);
		//if (VERBOSE) printf("deveice Selected returned %d\n", deviceSelected);
		int code = eSPAEAWB_EnumDevice(&devCount);
		eSPAEAWB_SelectDevice(0);
		eSPAEAWB_SetSensorType(1);
		if (VERBOSE) printf("dev count = %d\n", devCount);
		WCHAR myName[255];
		WCHAR targetName[255] = L"Touch+ Camera";
		int deviceWasSelected = 0;
		for (int i = 0; i < devCount; i++){
			eSPAEAWB_GetDevicename(i, myName, 255);
			wcout << myName << endl;
			myName[14] = 0;
			BOOL found = true;
			int j;
			for (j = 0; j < 13; j++){
				//	printf("t=%d  m=%d\n", targetName[j], myName[j]);
				if (myName[j] != targetName[j]){
					found = false;
				}
			}
			if (found){
				eSPAEAWB_SelectDevice(i);
				eSPAEAWB_SetSensorType(1);
				deviceWasSelected = 1;
				cout << "Touch+ Camera found" << endl;
				void *pHandle = NULL;
				ETRONDI_STREAM_INFO sinfo0[64];
				ETRONDI_STREAM_INFO sinfo1[8];
				int nCount0, nCount1, nRet;
				//
				EtronDI_Init(&pHandle);
				//
				if (EtronDI_FindDevice(pHandle)) {
					nRet = EtronDI_GetDeviceResolutionList(pHandle,
						64, sinfo0, 8, sinfo1);
					if (nRet >= 0) {
						nCount0 = (nRet >> 8) & 0xFF;
					}
				}
				EtronDI_Release(&pHandle);
			}

		}
		if (!deviceWasSelected){
			printf("Did not find a Touch+ Camera\n");
			return 0;
		}
		return 1;
	}
	int MyTouchFuncs::do_software_unlock(){

		int present = isCameraPresent();


		//eSPAEAWB_SelectDevice(0);
		//eSPAEAWB_SetSensorType(1); 
		int retVal = eSPAEAWB_SWUnlock(0x0107);
		if (VERBOSE) printf("software lock returned %d\n", retVal);
		retVal = eSPAEAWB_DisableAE();
		if (VERBOSE) printf("disable AE returned %d\n", retVal);
		retVal = eSPAEAWB_DisableAWB();
		if (VERBOSE) printf("disable AWB returned %d\n", retVal);
		return present;
	}
	int MyTouchFuncs::setExposureTime(int whichSide, float expTime)
	{
		int retCode = eSPAEAWB_SetExposureTime(whichSide, expTime);
		return retCode;
	}
	float MyTouchFuncs::getExposureTime(int whichSide)
	{
		float eTime = -1.0;
		int retCode = eSPAEAWB_GetExposureTime(whichSide, &eTime);
		return eTime;
	}
	int  MyTouchFuncs::setGlobalGain(int whichSide, float gain){
		return eSPAEAWB_SetGlobalGain(whichSide, gain);
	}

	float  MyTouchFuncs::getGlobalGain(int whichSide){
		float globalGain = -1.0;
		eSPAEAWB_GetGlobalGain(whichSide, &globalGain);
		return globalGain;

	}
	int  MyTouchFuncs::turnLEDsOn(){

		BYTE gpio_code;
		fprintf(stderr, "turning leds on\n");
		int retCode = eSPAEAWB_GetGPIOValue(1, &gpio_code);
		gpio_code |= 0x08;
		retCode = eSPAEAWB_SetGPIOValue(1, gpio_code);
		return retCode;
	}
	int  MyTouchFuncs::turnLEDsOff(){

		BYTE gpio_code;
		fprintf(stderr, "turning leds off\n");
		int retCode = eSPAEAWB_GetGPIOValue(1, &gpio_code);
		gpio_code &= 0xf7;
		retCode = eSPAEAWB_SetGPIOValue(1, gpio_code);
		return retCode;
	}

	int  MyTouchFuncs::getAccelerometerValues(int *x, int *y, int *z){

		return eSPAEAWB_GetAccMeterValue(x, y, z);
	}

	int	 MyTouchFuncs::setColorGains(int whichSide, float red, float green, float blue){
		return eSPAEAWB_SetColorGain(whichSide, red, green, blue);
	}

	int	 MyTouchFuncs::getColorGains(int whichSide, float *red, float *green, float * blue){
		return eSPAEAWB_GetColorGain(whichSide, red, green, blue);
	}

	int  MyTouchFuncs::enableAutoExposure(int whichSide){
		eSPAEAWB_SelectDevice(whichSide);
		return eSPAEAWB_EnableAE();
	}
	int  MyTouchFuncs::disableAutoExposure(int whichSide){
		eSPAEAWB_SelectDevice(whichSide);
		return eSPAEAWB_DisableAE();
	}
	int  MyTouchFuncs::enableAutoWhiteBalance(int whichSide){
		eSPAEAWB_SelectDevice(whichSide);
		return eSPAEAWB_EnableAWB();
	}
	int  MyTouchFuncs ::disableAutoWhiteBalance(int whichSide){
		eSPAEAWB_SelectDevice(whichSide);
		return eSPAEAWB_DisableAWB();
	}
	int MyTouchFuncs::startVideoStream(int width, int height, int framerate, int format, FRAME_CB_FUNC f )
	{
		do_software_unlock();
		ds_camera_ = new CCameraDS();
		int camera_count = CCameraDS::CameraCount();
		char camera_name[255] = { 0 };
		char camera_vid[10] = { 0 };
		char camera_pid[10] = { 0 };
		int i = 0, touchCameraId = -1;

		while (i < camera_count)
		{
			CCameraDS::CameraInfo(i, camera_vid, camera_pid);

			// VID PID is more reasonable
			if (0 == strncmp(camera_vid, TP_CAMERA_VID, 4) &&
				0 == strncmp(camera_pid, TP_CAMERA_PID, 4))
			{
				touchCameraId = i;
				break;
			}
			i++;
		}

		if (-1 == touchCameraId)
		{
			return false;
		}
		const int fmt = 1;
		bool retV = ds_camera_->OpenCamera(touchCameraId, format, width, height, framerate, f);
		fprintf(stderr, "open camera returned %d\n", retV);

	}
}

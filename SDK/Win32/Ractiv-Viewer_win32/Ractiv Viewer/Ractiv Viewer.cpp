// Ractiv Viewer.cpp
// created Feb 5th 2014 by Corey Manders
// 
// Intended to demonstrate basic control of the Touch+ module
// corey@ractiv.com

#include "stdafx.h"
#include "TouchPlus.h"
#include <iostream>
#include <windows.h> 
#include "turbojpeg.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define WIDTH 1280
#define HEIGHT 480
#define FRAMERATE 60
#define COMPRESSION MJPEG

/*
#define WIDTH 640
#define HEIGHT 480
#define FRAMERATE 30
#define COMPRESSION UNCOMPRESSED
*/
unsigned char * buffer;
unsigned char * reordered_buffer;

using namespace std;
using namespace cv;

Mat image(HEIGHT, WIDTH, CV_8UC3, Scalar(0, 0, 0));
bool leds_on = false;
bool use_ae = false;
bool use_awb = false;

volatile bool continue_processing = true;

tjhandle handle = tjInitDecompress();
void YUY2_to_RGB24_Microsoft(BYTE *pSrc, BYTE *pDst, int cx, int cy);

void process_key_switches(char key){

	// quit running if 'esc' is pressed
	if (key == 27) continue_processing = false;


	//touch plus exposure controls
	else if (key == 'e'){
		float expTime = TouchFuncs::MyTouchFuncs::getExposureTime(SIDE_LEFT);
		expTime += 1.0;
		TouchFuncs::MyTouchFuncs::setExposureTime(SIDE_BOTH, expTime);
		expTime = TouchFuncs::MyTouchFuncs::getExposureTime(SIDE_LEFT);
		printf("exposure time is: %g ms\n", expTime);
	}

	else if (key == 'w'){
		float expTime = TouchFuncs::MyTouchFuncs::getExposureTime(SIDE_LEFT);
		expTime -= 1.0;
		if (expTime <= 1.0) expTime = 1.0f;
		TouchFuncs::MyTouchFuncs::setExposureTime(SIDE_BOTH, expTime);
		expTime = TouchFuncs::MyTouchFuncs::getExposureTime(SIDE_LEFT);
		printf("exposure time is: %g ms\n", expTime);
	}


	//touch plus gain controls
	else if (key == 'g'){
		float globalGain = TouchFuncs::MyTouchFuncs::getGlobalGain(SIDE_LEFT);
		globalGain += 0.5;
		TouchFuncs::MyTouchFuncs::setGlobalGain(SIDE_BOTH, globalGain);
		globalGain = TouchFuncs::MyTouchFuncs::getGlobalGain(SIDE_LEFT);
		printf("global gain is: %g\n", globalGain);
	}
	else if (key == 'f'){
		float globalGain = TouchFuncs::MyTouchFuncs::getGlobalGain(SIDE_LEFT);
		globalGain -= 0.5;
		if (globalGain < 0.0)globalGain = 0.0;
		TouchFuncs::MyTouchFuncs::setGlobalGain(SIDE_BOTH, globalGain);
		globalGain = TouchFuncs::MyTouchFuncs::getGlobalGain(SIDE_LEFT);
		printf("global gain is: %g\n", globalGain);
	}

	// tooggle IR LEDs
	else if (key == 'l'){
		leds_on = !leds_on;
		if (leds_on) TouchFuncs::MyTouchFuncs::turnLEDsOn();
		else TouchFuncs::MyTouchFuncs::turnLEDsOff();
	}

	//print accelerometer data
	else if (key == 'a'){
		int x = -1, y = -1, z = -1;
		TouchFuncs::MyTouchFuncs::getAccelerometerValues(&x, &y, &z);
		printf("accelerometer data x=%d  y=%d  z=%d\n", x, y, z);
	}
	
	//color gains
	else if (key == 'i'){ //red
		float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		r_gain += 0.1;
		TouchFuncs::MyTouchFuncs::setColorGains(SIDE_BOTH,r_gain, g_gain, b_gain);
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
	}
	else if (key == 'u'){
		float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		r_gain -= 0.1;
		if (r_gain < 0.0)r_gain = 0.0;
		TouchFuncs::MyTouchFuncs::setColorGains(SIDE_BOTH,r_gain, g_gain, b_gain);
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
	}
	else if (key == 'k'){//green
		float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		g_gain += 0.1;
		TouchFuncs::MyTouchFuncs::setColorGains(SIDE_BOTH,r_gain, g_gain, b_gain);
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
	}
	else if (key == 'j'){

		float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		g_gain -= 0.1;
		if (g_gain < 0.0)g_gain = 0.0;
		TouchFuncs::MyTouchFuncs::setColorGains(SIDE_BOTH,r_gain, g_gain, b_gain);
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
	}
	else if (key == 'm'){// blue
		float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		b_gain += 0.1;
		TouchFuncs::MyTouchFuncs::setColorGains(SIDE_BOTH,r_gain, g_gain, b_gain);
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
	}
	else if (key == 'n'){
		float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		b_gain -= 0.1;
		if (b_gain < 0.0)b_gain = 0.0;
		TouchFuncs::MyTouchFuncs::setColorGains(SIDE_BOTH,r_gain, g_gain, b_gain);
		TouchFuncs::MyTouchFuncs::getColorGains(SIDE_LEFT,&r_gain, &g_gain, &b_gain);
		printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
	}
	
	else if (key == 'p'){
		use_ae = !use_ae;
		if (use_ae){
			TouchFuncs::MyTouchFuncs::enableAutoExposure(SIDE_BOTH);
			printf("Auto Exposure: ON\n");
		}
		else {
			TouchFuncs::MyTouchFuncs::disableAutoExposure(SIDE_BOTH);
			printf("Auto Exposure: OFF\n");
		}
	}

	else if (key == 'o'){
		use_awb = !use_awb;
		if (use_awb){
			TouchFuncs::MyTouchFuncs::enableAutoWhiteBalance(SIDE_BOTH);
			printf("Auto White Balance: ON\n");
		}
		else {
			TouchFuncs::MyTouchFuncs::disableAutoWhiteBalance(SIDE_BOTH);
			printf("Auto White Balance: OFF\n");
		}
	}

}

static void frameCallback(BYTE * pBuffer, long lBufferSize)
{
	char key;
	if (buffer)
	{
		if (COMPRESSION == MJPEG){

			//turbo jpeg decompress the frame
			tjDecompress2(handle, pBuffer, lBufferSize, buffer, WIDTH, WIDTH * 3, HEIGHT, TJPF_BGR, 0);
			
		}
		else {
			YUY2_to_RGB24_Microsoft(pBuffer, buffer, WIDTH, HEIGHT);
		}
		image.data = buffer;
	
		flip(image, image, -1);
		imshow("Touch Plus View", image);	
		key = waitKey(1);
		process_key_switches(key);
	}
}

void printInstructions(){
	printf("----------------------------\n");
	printf("   Instructions             \n");
	printf("----------------------------\n");
	printf("e/w  increase/decrease exposure time\n");
	printf("g/f  increase/decrease global gain\n");
	printf("l    toggle LEDs on and off\n");
	printf("a    print out accelerometer values\n");
	printf("i/u  increase/decrease red color gain\n");
	printf("k/j  increase/decrease green color gain\n");
	printf("m/n  increase/decrease blue color gain\n");
	printf("p	 enable/disable auto exposure\n");
	printf("o    enable/disable auto white balance\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
	printInstructions();

	// set some initial touchplus parameters
	TouchFuncs::MyTouchFuncs::do_software_unlock();
	TouchFuncs::MyTouchFuncs::startVideoStream(WIDTH, HEIGHT, FRAMERATE, COMPRESSION, frameCallback);
	TouchFuncs::MyTouchFuncs::turnLEDsOff();
	TouchFuncs::MyTouchFuncs::disableAutoExposure(SIDE_BOTH);
	TouchFuncs::MyTouchFuncs::disableAutoWhiteBalance(SIDE_BOTH);

	buffer = (unsigned char*)malloc(WIDTH*HEIGHT * 3);

	while (continue_processing);
	return 0;
}

void YUY2_to_RGB24_Microsoft(BYTE *pSrc, BYTE *pDst, int cx, int cy)
{
	int nSrcBPS, nDstBPS, x, y, x2, x3, m;
	int ma0, mb0, m02, m11, m12, m21;
	BYTE *pS0, *pD0;
	int Y, U, V, Y2;
	BYTE R, G, B, R2, G2, B2;
	//
	nSrcBPS = cx * 2;
	nDstBPS = ((cx * 3 + 3) / 4) * 4;
	//
	pS0 = pSrc;
	pD0 = pDst + nDstBPS*(cy - 1);
	for (y = 0; y<cy; y++) {
		for (x3 = 0, x2 = 0, x = 0; x<cx; x += 2, x2 += 4, x3 += 6) {
			Y = (int)pS0[x2 + 0] - 16;
			Y2 = (int)pS0[x2 + 2] - 16;
			U = (int)pS0[x2 + 1] - 128;
			V = (int)pS0[x2 + 3] - 128;
			//
			ma0 = 298 * Y;
			mb0 = 298 * Y2;
			m02 = 409 * V + 128;
			m11 = -100 * U;
			m12 = -208 * V + 128;
			m21 = 516 * U + 128;
			//
			m = (ma0 + m02) >> 8;
			R = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (ma0 + m11 + m12) >> 8;
			G = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (ma0 + m21) >> 8;
			B = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			//
			m = (mb0 + m02) >> 8;
			R2 = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (mb0 + m11 + m12) >> 8;
			G2 = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (mb0 + m21) >> 8;
			B2 = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			//
			pD0[x3] = B;
			pD0[x3 + 1] = G;
			pD0[x3 + 2] = R;
			pD0[x3 + 3] = B2;
			pD0[x3 + 4] = G2;
			pD0[x3 + 5] = R2;
		}
		pS0 += nSrcBPS;
		pD0 -= nDstBPS;
	}
}


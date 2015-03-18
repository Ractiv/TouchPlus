#ifndef _TOUCHPLUS_
#define _TOUCHPLUS_
#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <libuvc/libuvc.h>

#define VERBOSE 0
#define MJPEG 1
#define UNCOMPRESSED 0


void	init_camera();

int 	do_software_unlock();
int 	do_software_lock();

int 	enableAutoExposure();
int 	disableAutoExposure();
int 	enableAutoWhiteBalance();
int 	disableAutoWhiteBalance();
int     turnLEDsOn();
int     turnLEDsOff();

float 	getExposureTime();
int 	setExposureTime(float etime);
int 	getGlobalGain();
int 	setGlobalGain(int ggain);
int 	getAccelerometerValues( int *x, int *y, int *z);
int 	read_Flash(unsigned char * data, int length);
int 	write_Flash(unsigned char * data, int length);
int 	startVideoStream(int width, int height, int framerate, int format, void (*f)(uvc_frame_t * frame, void * ptr));
int 	stop_VideoStream();
long 	getCurrentFrame();

// UVC accessor
int 	getAEMode();
int 	setAEMode(int mode);

int     setColorGains(float red, float green, float blue);
int     getColorGains(float *red, float *green, float *blue);

unsigned char * getDataPointer();


#endif
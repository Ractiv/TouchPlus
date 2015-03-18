#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include "TouchPlus.h"
//#include <libusb.h>
//#include <libuvc/libuvc.h>

#define MJPEG 1
#define UNCOMPRESSED 0

#define WIDTH 1280
#define HEIGHT 480
#define FRAMERATE 60
#define COMPRESSION MJPEG

using namespace cv;
using namespace std;

Mat image(HEIGHT, WIDTH, CV_8UC3, Scalar(0, 0, 0));
bool leds_on = false;
bool use_ae = false;
bool use_awb = false;

volatile bool continue_processing = true;

unsigned char *buffer;
int decompressJPEG = 0;
uvc_frame_t *bgr;
unsigned char * liveData;
volatile long currentFrame = 0;
int mode = 0;

void process_key_switches(char key){
// quit running if 'esc' is pressed
 
  if (key == 27) continue_processing = false;


  //touch plus exposure controls
  else if (key == 'e'){
    float expTime = getExposureTime();
    expTime += 1.0;
    setExposureTime(expTime);
    expTime = getExposureTime();
    printf("exposure time is: %g ms\n", expTime);
  }

  else if (key == 'w'){
    float expTime = getExposureTime();
    expTime -= 4.0;
    if (expTime <= 1.0) expTime = 1.0f;
    printf("trying to set to %g\n",expTime);
    setExposureTime(expTime);
    expTime = getExposureTime();
    printf("exposure time is: %g ms\n", expTime);
  }

  //print accelerometer data
  else if (key == 'a'){
    int x = -1, y = -1, z = -1;
    getAccelerometerValues(&x, &y, &z);
    printf("accelerometer data x=%d  y=%d  z=%d\n", x, y, z);
  }

  //enable/disable autoexposure
  else if (key == 'p'){
    use_ae = !use_ae;
    if (use_ae){
      enableAutoExposure();
      printf("Auto Exposure: ON\n");
    }
    else {
      disableAutoExposure();
      printf("Auto Exposure: OFF\n");
    }
  }
  else if (key == '['){
    mode = getAEMode();
    printf("auto-exposure mode = %d\n",mode);
  }
  else if (key == ']'){
    mode = getAEMode();
    mode = 0;
    if (mode<0)mode = 0;
    setAEMode(mode);
    mode = getAEMode();
     printf("current mode = %d\n",mode );
  }
  else if (key == '\\'){
    mode = getAEMode();
    mode=4;
  
    setAEMode(mode);
    mode = getAEMode();
    printf("current mode = %d\n",mode );
  }

  else if (key == 'o'){
    use_awb = !use_awb;
    if (use_awb){
      enableAutoWhiteBalance();
      printf("Auto White Balance: ON\n");
    }
    else {
      disableAutoWhiteBalance();
      printf("Auto White Balance: OFF\n");
    }
  }

  //global gain controls
  else if (key == 'g'){
    int globalGain = getGlobalGain();
    globalGain += 1;
    setGlobalGain(globalGain);
    globalGain = getGlobalGain();
    printf("global gain is: %d\n", globalGain);
  }
  else if (key == 'f'){
    int globalGain = getGlobalGain();
    globalGain -= 1;
    if (globalGain < 0)globalGain = 0;
    setGlobalGain(globalGain);
    globalGain = getGlobalGain();
    printf("global gain is: %d\n", globalGain);
  }

  


  // tooggle IR LEDs
  else if (key == 'l'){
    leds_on = !leds_on;
    if (leds_on) turnLEDsOn();
    else turnLEDsOff();
  }


  
  //color gains
  else if (key == 'i'){ //red
    float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
    getColorGains(&r_gain, &g_gain, &b_gain);
    r_gain += 0.1;
    setColorGains(r_gain, g_gain, b_gain);
    getColorGains(&r_gain, &g_gain, &b_gain);
    printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
  }
  else if (key == 'u'){
    float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
    getColorGains(&r_gain, &g_gain, &b_gain);
    r_gain -= 0.1;
    if (r_gain < 0.0)r_gain = 0.0;
    setColorGains(r_gain, g_gain, b_gain);
    getColorGains(&r_gain, &g_gain, &b_gain);
    printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
  }
  else if (key == 'k'){//green
    float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
    getColorGains(&r_gain, &g_gain, &b_gain);
    g_gain += 0.1;
    setColorGains(r_gain, g_gain, b_gain);
    getColorGains(&r_gain, &g_gain, &b_gain);
    printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
  }
  else if (key == 'j'){

    float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
    getColorGains(&r_gain, &g_gain, &b_gain);
    g_gain -= 0.1;
    if (g_gain < 0.0)g_gain = 0.0;
    setColorGains(r_gain, g_gain, b_gain);
    getColorGains(&r_gain, &g_gain, &b_gain);
    printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
  }
  else if (key == 'm'){// blue
    float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
    getColorGains(&r_gain, &g_gain, &b_gain);
    b_gain += 0.1;
    setColorGains(r_gain, g_gain, b_gain);
    getColorGains(&r_gain, &g_gain, &b_gain);
    printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
  }
  else if (key == 'n'){
    float r_gain = -1.0, b_gain = -1.0, g_gain = -1.0;
    getColorGains(&r_gain, &g_gain, &b_gain);
    b_gain -= 0.1;
    if (b_gain < 0.0)b_gain = 0.0;
    setColorGains(r_gain, g_gain, b_gain);
    getColorGains(&r_gain, &g_gain, &b_gain);
    printf("color gain r: %g  g: %g  b: %g\n", r_gain, g_gain, b_gain);
  }
  


  
}


void cb(uvc_frame_t *frame, void *ptr) {


  uvc_error_t ret;

  if (COMPRESSION == UNCOMPRESSED){
    ret = uvc_any2bgr(frame, bgr);
  }
  else {
    //fprintf(stderr,"callback called\n");
    ret = uvc_mjpeg2rgb(frame, bgr);

    
    if (ret) {
      uvc_perror(ret, "change to BGR error");
      uvc_free_frame(bgr);
      return;
    }
    unsigned char tmp;
  
   // fprintf(stderr,"height = %d width = %d\n",frame->width,frame->height);
    image.data = (uchar*)bgr->data;
    cvtColor(image,image,COLOR_BGR2RGB);
    imshow("Ractiv Viewer",image);

    char key =waitKey(5);
    

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
  printf("p  enable/disable auto exposure\n");
  printf("o    enable/disable auto white balance\n");
}


int main(int ac, char** av)
{
    printInstructions();
    
    
    bgr = uvc_allocate_frame(WIDTH*HEIGHT*3);
    fprintf(stderr,"about to start streaming\n");
 
    init_camera();
   // 
    startVideoStream(WIDTH,HEIGHT,60,MJPEG,cb);
    int x,y,z;
    //get_AccelerometerValues(&x,&y,&z);
    fprintf(stderr,"x= %d y= %d z= %d\n",x,y,z);
    //turnLEDsOff();
    //disableAutoExposure();
    //disableAutoWhiteBalance();
    namedWindow("Ractiv Viewer",1);
    //setExposureTime(1);
    while(continue_processing){
    //   turnLEDsOff();
    //  waitKey(1000);
    //   turnLEDsOn();
        
       char key = waitKey(10);
       //fprintf(stderr,"key = %d\n",key);
       if (key>0)process_key_switches(key);
	/*
    //	inputVideo >> inputImage;
      //while(getCurrentFrame()<=shownFrame)usleep(5);

    	//rectangle(rview_plus640,p1,p2,Scalar(0,255,0),8,0);
    	imshow("image Left", inputImage);
        //shownFrame = getCurrentFrame();
    	//imshow("original image Right", imageROI_2);
    	//imshow("image Right", inputImage(roi2));

    
    	char c = (char)waitKey(10);
        if (c == 27) break;
        else if (c=='g'){
          ggain +=0.1;
          setGlobalGain(ggain);
            
        }
        else if (c=='d'){
            ggain -=0.3;
            if (ggain<0.0) ggain = 0.0;
            printf("decreasing gain\n");
            setGlobalGain(ggain);
        }
        else if (c=='e'){
          enable_ae();
          
         // uvc_error_t error = uvc_set_ae_mode(devh, 0);
         // printf("auto exposure on error =%d\n",error);
        }
        else if (c=='w'){
          disable_ae();
        }
        else if (c=='s'){
          enable_awb();
        }
        else if (c=='a'){
          disable_awb();
        }
        else if (c=='x'){
          expTime += 1.0;
          setExposureTime(expTime);
          printf("Exposure time = %2.2f\n",expTime);
        }
        else if (c=='z'){
          expTime -=1.0;
          if (expTime <= 0.0)expTime=0.0;
          setExposureTime(expTime);
          printf("Exposure time = %2.2f\n",expTime);
        }
        else if (c=='c'){
           float newExp = getExposureTime();
           printf("Exposure time = %2.2f\n",newExp); 
        }
        else if (c=='/'){
          int x,y,z;
          getAccelerometerValues(&x,&y,&z);
          printf("x = %d\t y = %d\t z = %d\n",x,y,z);
        }
        else if (c=='r'){
          unsigned char data[15];
          readFlash(data,15);
          for (int i=0;i<15; i++) printf("data[%u]-> %u\n",i,data[i]);
        }
        else if (c=='t'){
          unsigned char data[15];
          for (int i=0;i<15;i++) data[i]=(unsigned char)0xde;
          writeFlash(data,15);  
        }
	*/
    }
    stop_VideoStream();

}

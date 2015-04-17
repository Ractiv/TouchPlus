#include "TouchPlus.h"

using namespace std;


// evil global variables I would like to get rid of




//uvc_error_t res;
uvc_context_t * ctx = NULL;
uvc_device_handle_t* devh = NULL;
uvc_device_t *dev;
static libusb_device_handle * dev_handle;




void printExtensionUnits(const uvc_extension_unit_t * extu){
    printf("unit ID = %d\n",extu->bUnitID);
    printf("contols bitmap = %llx\n",extu->bmControls);
    for (int i = 0;i<16;i++){
        printf("extension code [%d] ==> 0x%x\n",i,extu->guidExtensionCode[i]);
    }
    const uvc_extension_unit_t *next = extu->next;
    if (next) printExtensionUnits(next);

}


void init_camera(){
   
    uvc_error_t res;
    uvc_device_t *dev;
   
    res = uvc_init(&ctx, NULL);
    if (res < 0) {
        uvc_perror(res, "uvc_init");
        return ;
    }
    puts("UVC initialized");
    res = uvc_find_device(ctx, &dev,0x1e4e, 0x0107, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
    if (res < 0) {
        uvc_perror(res, "uvc_find_device"); /* no devices found */
    } else {
        puts("Device found");
        res = uvc_open(dev, &devh);
        if (VERBOSE) uvc_print_diag(devh, stderr);
    }

    const uvc_extension_unit_t *extu = uvc_get_extension_units(devh);
 //   printExtensionUnits(extu);

 
    fprintf(stderr,"returning libusb handle\n");
    dev_handle = uvc_get_libusb_handle(devh);
    fprintf(stderr,"Handle returned\n");
    do_software_unlock();
   //startVideoStream(640, 240, 30, 0);
 
}


void printerror(int r){
    if (r==LIBUSB_ERROR_TIMEOUT){
        fprintf(stderr,"TIME-OUT occured\n");
    }
    else if (r==LIBUSB_ERROR_PIPE){
        fprintf(stderr,"control request was not supported by the device\n");
    }
    else if (r==LIBUSB_ERROR_NO_DEVICE)
    {
        fprintf(stderr,"the device has been disconnected\n");
    }
}


void read_ADDR_85(libusb_device_handle *dev_handle, unsigned short wValue = 0x0300){
    
    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char data[2];
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,0x85,wValue,wIndex,data,2,10000 );
    if (VERBOSE){
        cout <<"get (0x85) (0x"<<hex<<wValue<<")returned "<<r<<endl;
        for (int i = 0;i<2; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    
    
}
unsigned char read_ADDR_81(libusb_device_handle *dev_handle, unsigned short wValue= 0x0300, int length = 4){
    
    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char data[length];
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,0x81,wValue,wIndex,data,length,10000 );
    if (VERBOSE){
        cout <<"get (0x81) returned "<<r<<endl;
        for (int i = 0;i<length; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    return data[1];
}
unsigned char read_ADDR_81(libusb_device_handle *dev_handle,unsigned char * data, unsigned short wValue= 0x0300, int length = 4){
    
    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_get = 0xa1;
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,0x81,wValue,wIndex,data,length,10000 );
    if (VERBOSE){
        cout <<"get (0x81) returned "<<r<<endl;
        for (int i = 0;i<length; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    return 0;
}


void write_ADDR_01(libusb_device_handle *dev_handle, unsigned char * data,unsigned short wValue = 0x0300, int length=4)
{
    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_set = 0x21;
    int r= libusb_control_transfer(dev_handle,bmRequestType_set,0x01,wValue,wIndex,data,length,10000 );
    if (VERBOSE){
        cout <<"set (0x01) returned "<<r<<endl;
        for (int i = 0;i<length; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    
}

int  startVideoStream(int width, int height, int framerate, int format, void (*f)(uvc_frame_t * frame, void * ptr)){ 
    
    int r;
   
    //get video stream begin
    uvc_stream_ctrl_t ctrl;
    uvc_error_t res;
    fprintf(stderr,"start streaming!!!\n");
    
    
    
    if (format == UNCOMPRESSED){
        res = uvc_get_stream_ctrl_format_size(
                                              devh, &ctrl, /* result stored in ctrl */
                                              UVC_FRAME_FORMAT_YUYV, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
                                              width, height, framerate /* width, height, fps */
                                              );
    }
    else if (format == MJPEG){

        res = uvc_get_stream_ctrl_format_size(
                                              devh, &ctrl, /* result stored in ctrl */
                                              UVC_FRAME_FORMAT_MJPEG, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
                                              width, height, framerate /* width, height, fps */
                                              );
    }
    
    
    /* Print out the result */
    // uvc_print_stream_ctrl(&ctrl, stderr);
    if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
    }
    else {
        int value = 0;
       

        //  inputImage.data = (unsigned char*)bgr->data;
         res = uvc_start_streaming(devh, &ctrl, f, (void*)(&value), 0);
        if (res < 0) {
            uvc_perror(res, "start_streaming error, unable to start stream"); /* unable to start stream */
        }
        else {
            puts("Streaming...");
        }
    }
    return 0;
}

int stop_VideoStream(){
    uvc_stop_streaming(devh);
    uvc_close(devh);
    // uvc_unref_device(dev);
    uvc_exit(ctx);
    return 0;
}

int write_Flash(unsigned char * data, int length){
    
    
    unsigned char count[1];
    read_ADDR_81(dev_handle,count,0x0a00,1);
    
    read_ADDR_85(dev_handle,0x0b00);
    unsigned char mystery[16]= {count[0],0x81,0x05,0x00,0x00,0xc8,0x00,0x00,0x00,(unsigned char)length,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char mystery2[16]={count[0],0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    write_ADDR_01(dev_handle,mystery,0x0b00,16);
    
    read_ADDR_85(dev_handle,0x0c00);
    write_ADDR_01(dev_handle,data,0x0c00,length);
    
    read_ADDR_85(dev_handle,0x0b00);
    write_ADDR_01(dev_handle,mystery2,0x0b00,16);
    
    read_ADDR_85(dev_handle,0x0a00);
    
    write_ADDR_01(dev_handle,count,0x0a00,1);
    //closeTouchPlusDevHandle(dev_handle);
    return 0;
}


int read_Flash(unsigned char * data, int length){
    
    read_ADDR_85(dev_handle,0x0a00);
    unsigned char count[1];
    read_ADDR_81(dev_handle,count,0x0a00,1);
    read_ADDR_85(dev_handle,0x0b00);
    unsigned char mystery[16]= {count[0],0x41,0x05,0x00,0x00,0xc8,0x00,0x00,0x00,(unsigned char)length,0x00,0x00,0x00,0x00,0x00,0x00};
    write_ADDR_01(dev_handle,mystery,0x0b00,16);
    
    read_ADDR_85(dev_handle,0x0c00);
    read_ADDR_81(dev_handle,data,0x0c00,length);
    
    
    write_ADDR_01(dev_handle,count,0x0a00,1);
    // closeTouchPlusDevHandle(dev_handle);
    return 0;
}



int getAccelerometerValues(int *x, int *y, int *z)
{
    
    read_ADDR_85(dev_handle);
    unsigned char data[4];
    data[0] = 0xa0;
    data[1] = 0x80;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_x_1 =read_ADDR_81(dev_handle);
    // printf("val 0 = %u\n", acc_x_1);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0x81;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_x_2 =read_ADDR_81(dev_handle);
    //  printf("val 1 = %u\n", acc_x_2);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0x82;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_y_1 =read_ADDR_81(dev_handle);
    //  printf("val 2 = %u\n", acc_y_1);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0x83;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_y_2=read_ADDR_81(dev_handle);
    //printf("val 3 = %u\n", acc_y_2);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0x84;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_z_1 =read_ADDR_81(dev_handle);
    //printf("val 4 = %u\n", acc_z_1);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0x85;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_z_2 =read_ADDR_81(dev_handle);
    //printf("val 6 = %u\n", acc_z_2);
    
    *x= (int)acc_x_1;
    *y= (int)acc_y_1;
    *z= (int)acc_z_1;
    
    return 0;
}

int     getAEMode(){
        uint8_t mode = 0;
        //
        int length = uvc_get_ctrl_len(devh, 3, 3);
        printf("length = %d\n", length);
       // uvc_error_t err = uvc_get_ae_mode(devh, &mode, UVC_GET_CUR);
        return (int)mode;
}
int     setAEMode(int mode){
        uvc_error_t err = uvc_set_ae_mode(devh, mode);
        return (int)err;
}

int  getGlobalGain(){
    uint16_t gain = 0;
    
    uvc_error_t err =  uvc_get_gain(devh, &gain, UVC_GET_CUR);
    return gain;
    /*
    read_ADDR_85(dev_handle,0x0200);
    unsigned char data[6];
    data[0] = 0x99;
    data[1] = 0x42;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    unsigned char gain =read_ADDR_81(dev_handle,0x0200,6);
    printf("gain = %x",gain);
    float toReturn = (float)gain;
    return toReturn/7.75;
    */
}

int setGlobalGain(int ggain){
    
    uvc_set_gain(devh, ggain);
    /*
    read_ADDR_85(dev_handle,0x0200);
    unsigned char data[6];
    int toSet = (int)(ggain*7.75);
    if (toSet > 255 )toSet = 255;
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x00;
    data[3] = toSet;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);
    */
    return 0;
}
int  setColorGains(float red, float green, float blue){
    
    unsigned char data[4];
    read_ADDR_85(dev_handle);

    //set red
    int toSet = (red *64);
    if (toSet<0)toSet =0;
    if (toSet>255)toSet = 255;
    data[0] = 0x02;
    data[1] = 0xf1;
    data[2] = 0xcc;
    data[3] = (unsigned char)toSet;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    data[0] = 0x02;
    data[1] = 0xf2;
    data[2] = 0x8c;
    data[3] = (unsigned char)toSet;
     read_ADDR_85(dev_handle);
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);
    
 

 /// green
    int toSet2 = (green *64);
    if (toSet2<0)toSet2 =0;
    if (toSet2>255)toSet2 = 255;
    data[0] = 0x02;
    data[1] = 0xf1;
    data[2] = 0xcd;
    data[3] = (unsigned char)toSet2;
     read_ADDR_85(dev_handle);
     write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    data[0] = 0x02;
    data[1] = 0xf2;
    data[2] = 0x8d;
    data[3] = (unsigned char)toSet2;
     read_ADDR_85(dev_handle);
     write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    data[0] = 0x02;
    data[1] = 0xf1;
    data[2] = 0xce;
    data[3] = (unsigned char)toSet2;
     read_ADDR_85(dev_handle);
     write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    data[0] = 0x02;
    data[1] = 0xf2;
    data[2] = 0x8e;
    data[3] = (unsigned char)toSet2;
     read_ADDR_85(dev_handle);
     write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);




     toSet2 = (green *64);
    if (toSet2<0)toSet2 =0;
    if (toSet2>255)toSet2 = 255;
    data[0] = 0x02;
    data[1] = 0xf1;
    data[2] = 0xce;
    data[3] = (unsigned char)toSet2;
     read_ADDR_85(dev_handle);
     write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    
    //blue side 1
    int toSet3 = (blue *64);
    if (toSet3<0)toSet3 =0;
    if (toSet3>255)toSet3 = 255;
    data[0] = 0x02;
    data[1] = 0xf1;
    data[2] = 0xcf;
    data[3] = (unsigned char)toSet3;
     read_ADDR_85(dev_handle);
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    data[0] = 0x02;
    data[1] = 0xf2;
    data[2] = 0x8f;
    data[3] = (unsigned char)toSet3;
     read_ADDR_85(dev_handle);
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);
    
    return 0;
}
int     getColorGains(float *red, float *green, float *blue){
    unsigned char data[4];
    read_ADDR_85(dev_handle);

    data[0] = 0x82;
    data[1] = 0xf1;
    data[2] = 0xcc;
    data[3] = 0X00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);
    float mred=(float)data[1];
    *red = mred/64;
    
   
  //  printf("red data = %x\n",data[1]);
    
    read_ADDR_85(dev_handle);
    data[0] = 0x82;
    data[1] = 0xf1;
    data[2] = 0xcd;
    data[3] = 0X00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);
  //  printf("green data = %x\n",data[1]);
    float mgreen= (float)data[1];
    *green = mgreen/64;
    
    read_ADDR_85(dev_handle);
    data[0] = 0x82;
    data[1] = 0xf1;
    data[2] = 0xcf;
    data[3] = 0X00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);
 //   printf("blue data = %x\n",data[1]);
    float mblue = (float)data[1];
    *blue = mblue/64;
    return 0;
}

int    turnLEDsOff()
{

    unsigned char bmRequestType_set = 0x21;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char GET_CUR = 0x85;
    unsigned char SET_CUR = 0x01;
    unsigned short wValue =0x0300;
    unsigned short wIndex = 0x0400;
    unsigned char data[5];
 
    read_ADDR_85(dev_handle);
    data[0]=0x82;
    data[1]=0xf0;
    data[2]=0x17;
    data[3]=0x00;
    write_ADDR_01(dev_handle,data);

    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);
    read_ADDR_85(dev_handle);

    data[0]=0x02;
    data[1]=0xf0;
    data[2]=0x17;
    data[3]=0x15;
    
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    return 0;
}
int     turnLEDsOn(){
    //libusb_transfer transfer ={0};
    unsigned char bmRequestType_set = 0x21;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char GET_CUR = 0x85;
    unsigned char SET_CUR = 0x01;
    unsigned short wValue =0x0300;
    unsigned short wIndex = 0x0400;
    unsigned char data[5];
    
    read_ADDR_85(dev_handle);   

    data[0]=0x82;
    data[1]=0xf0;
    data[2]=0x17;
    data[3]=0x00;
    
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);
    
    data[0]=0x02;
    data[1]=0xf0;
    data[2]=0x17;
    data[3]=0x1D;
    
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle,data);

    return 0;
}

int setExposureTime(float time){
    //convert to the time to integer
    unsigned short mytime = (unsigned short)(time*90.08);
    
    unsigned char data[6];
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x3b;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x33;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x37;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    unsigned char byte0 = (unsigned char)(mytime&0x00ff);
    unsigned char byte1 = (unsigned char)(mytime>>8);
    
    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x0f;
    data[3] = byte1;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);
    
    
    
    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x10;
    data[3] = byte0;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);
    
    
    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x2e;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);
    
    
    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x2d;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);
    
    return 0;
}

float getExposureTime(){
    
    unsigned char data[6];
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x3b;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);
    
    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x99;
    data[1] = 0x42;
    data[2] = 0x0f;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    
    
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    unsigned short temp1 =(unsigned short)read_ADDR_81(dev_handle,0x0200,6);
    
    
    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x99;
    data[1] = 0x42;
    data[2] = 0x10;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    
    
    write_ADDR_01(dev_handle,data,0x0200);
    read_ADDR_85(dev_handle,0x0200);
    unsigned short temp2 = (unsigned short)read_ADDR_81(dev_handle,0x0200,6);
    
    unsigned short tempdata = 0;
    tempdata +=temp1;
    tempdata = tempdata << 8;
    tempdata += temp2;
    float toReturn = (float)tempdata;
    toReturn /=90.08;
    return toReturn;
}





int disableAutoExposure(){
    
    unsigned char endpoint = 0x83;
    unsigned char bmRequestType_set = 0x21;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char GET_CUR = 0x85;
    unsigned char SET_CUR = 0x01;
    unsigned short wValue =0x0300;
    unsigned short wIndex = 0x0400;
    unsigned char data[5];
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x23;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    data[0] = 0x20;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    
    return 0;
    
}

int enableAutoExposure(){
    
    //  libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    printf("called\n");
    
    int r;
    
    unsigned char bmRequestType_set = 0x21;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char GET_CUR = 0x85;
    unsigned char SET_CUR = 0x01;
    unsigned short wValue =0x0300;
    unsigned short wIndex = 0x0400;
    unsigned char data[5];
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    if (VERBOSE){
        cout <<"set returned "<<r<<endl;
        for (int i = 0;i<4; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    if (VERBOSE) {
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<2; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    if (VERBOSE) {
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<4; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    GET_CUR = 0x85;
    if (VERBOSE) {
        r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<2; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x23;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    if (VERBOSE){
        cout <<"set returned "<<r<<endl;
        for (int i = 0;i<4; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    if (VERBOSE) {
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<2; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    if (VERBOSE){
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<4; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    if (VERBOSE) {
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<2; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    data[0] = 0x20;
    data[1] = 0xa2;
    data[2] = 0x01;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    if (VERBOSE) {
        cout <<"set returned "<<r<<endl;
        for (int i = 0;i<4; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    if (VERBOSE) {
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<2; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    if (VERBOSE) {
        cout <<"get returned "<<r<<endl;
        for (int i = 0;i<4; i++){
            printf("data[%d] = %x\n",i,data[i]);
        }
    }
    
    return 0;
}

int do_software_unlock(){
    
    int r = 0;
    //libusb_transfer transfer ={0};
    unsigned char bmRequestType_set = 0x21;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char GET_CUR = 0x85;
    unsigned char SET_CUR = 0x01;
    unsigned short wValue =0x0300;
    unsigned short wIndex = 0x0400;
    unsigned char data[5];
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;

    
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );

    data[0]=0x82;
    data[1]=0xf1;
    data[2]=0xf8;
    data[3]=0x00;
    
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    data[0]=0x00;
    data[1]=0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    
    data[0]=0x00;
    data[1]=0x00;
    data[2]=0x00;
    data[3]=0x00;
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    
    data[0]=0x00;
    data[1]=0x00;
    GET_CUR=0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000);
    
    data[0]=0x02;
    data[1]=0xf1;
    data[2]=0xf8;
    data[3]=0x40;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,0 );
    
    data[0]=0x00;
    data[1]=0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,0 );
    
    data[0]=0x00;
    data[1]=0x00;
    data[2]=0x00;
    data[3]=0x00;
    GET_CUR=0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,0 );

    fprintf(stderr,"Camera unlocked\n");
    /*
     r = libusb_release_interface(dev_handle, 0); //release the claimed interface
     if(r!=0) {
     cout<<"Cannot Release Interface"<<endl;
     return 1;
     }
     libusb_close(dev_handle); //close the device we opened
     libusb_exit(ctx); //needs to be called to end the
     */
    return 0;
}


int enableAutoWhiteBalance(){
    
    unsigned char bmRequestType_set = 0x21;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char GET_CUR = 0x85;
    unsigned char SET_CUR = 0x01;
    unsigned short wValue =0x0300;
    unsigned short wIndex = 0x0400;
    unsigned char data[5];
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    data[0] = 0x20;
    data[1] = 0xa8;
    data[2] = 0x00;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    data[0] = 0x20;
    data[1] = 0xa9;
    data[2] = 0x0C;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    data[0] = 0x20;
    data[1] = 0xaa;
    data[2] = 0x01;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    return 0;
    
}

int disableAutoWhiteBalance(){
    
    unsigned char bmRequestType_set = 0x21;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char GET_CUR = 0x85;
    unsigned char SET_CUR = 0x01;
    unsigned short wValue =0x0300;
    unsigned short wIndex = 0x0400;
    unsigned char data[5];
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    data[0] = 0x20;
    data[1] = 0xa8;
    data[2] = 0x00;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    
    data[0] = 0x20;
    data[1] = 0xa9;
    data[2] = 0x0C;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    }
    data[0] = 0x20;
    data[1] = 0xaa;
    data[2] = 0x00;
    data[3] = 0x00;
    r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"set returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    } 
    GET_CUR = 0x85;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
    } 
    GET_CUR = 0x81;
    r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
    cout <<"get returned "<<r<<endl;
    for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
    } 
    
    
    return 0;
    
}

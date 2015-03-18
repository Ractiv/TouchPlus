//////////////////////////////////////////////////////////////////////
// Video Capture using DirectShow
// Author: Shiqi Yu (shiqi.yu@gmail.com)
// Thanks to:
//		HardyAI@OpenCV China
//		flymanbox@OpenCV China (for his contribution to function CameraName, and frame width/height setting)
// Last modification: April 9, 2009
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// 使用说明：
//   1. 将CameraDS.h CameraDS.cpp以及目录DirectShow复制到你的项目中
//   2. 菜单 Project->Settings->Settings for:(All configurations)->C/C++->Category(Preprocessor)->Additional include directories
//      设置为 DirectShow/Include
//   3. 菜单 Project->Settings->Settings for:(All configurations)->Link->Category(Input)->Additional library directories
//      设置为 DirectShow/Lib
//////////////////////////////////////////////////////////////////////

#ifndef CCAMERA_H
#define CCAMERA_H

#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>
#include <windows.h>


#include <qedit.h>
#include <dshow.h>

#define MYFREEMEDIATYPE(mt)	{if ((mt).cbFormat != 0)		\
					{CoTaskMemFree((PVOID)(mt).pbFormat);	\
					(mt).cbFormat = 0;						\
					(mt).pbFormat = NULL;					\
				}											\
				if ((mt).pUnk != NULL)						\
				{											\
					(mt).pUnk->Release();					\
					(mt).pUnk = NULL;						\
				}}									


typedef void(*FRAME_CB_FUNC)(BYTE * pBuffer, long lBufferSize);



class CCamFrameHandler {
public:
    virtual void CamFrameData(double dblSampleTime, BYTE * pBuffer, long lBufferSize) = 0;
};


class CSampleGrabberCB : public ISampleGrabberCB
{
public:
    long                   lWidth;
    long                   lHeight;
    CCamFrameHandler    *  frame_handler;
    BOOL                   bGrabVideo;
    FRAME_CB_FUNC          m_frame_cb;
public:
    CSampleGrabberCB(FRAME_CB_FUNC fp = NULL){
        lWidth = 0;
        lHeight = 0;
        bGrabVideo = FALSE;
        frame_handler = NULL;
        m_frame_cb = fp;
    }
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv) {
        if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown){
            *ppv = (void *) static_cast<ISampleGrabberCB*> (this);
            return NOERROR;
        }
        return E_NOINTERFACE;
    }

    STDMETHODIMP SampleCB(double SampleTime, IMediaSample * pSample)  {
        return 0;
    }

    STDMETHODIMP BufferCB(double dblSampleTime, BYTE * pBuffer, long lBufferSize){
        if (NULL != m_frame_cb && NULL != pBuffer)
        {
            (*m_frame_cb)(pBuffer, lBufferSize);
        }
        return 0;
    }
};

class CCameraDS  
{
private:

	bool m_bConnected;
	int m_nWidth;
	int m_nHeight;
	bool m_bLock;
	bool m_bChanged;
	long m_nBufferSize;

    // peter
    CSampleGrabberCB*        m_CB;

	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<IBaseFilter> m_pDeviceFilter;
	CComPtr<IMediaControl> m_pMediaControl;
	CComPtr<IBaseFilter> m_pSampleGrabberFilter;
	CComPtr<ISampleGrabber> m_pSampleGrabber;
	CComPtr<IPin> m_pGrabberInput;
	CComPtr<IPin> m_pGrabberOutput;
	CComPtr<IPin> m_pCameraOutput;
	CComPtr<IMediaEvent> m_pMediaEvent;
	CComPtr<IBaseFilter> m_pNullFilter;
	CComPtr<IPin> m_pNullInputPin;

private:
	bool BindFilter(int nCamIDX, IBaseFilter **pFilter);
	void SetCrossBar();

public:
	CCameraDS();
	virtual ~CCameraDS();

	//打开摄像头，nCamID指定打开哪个摄像头，取值可以为0,1,2,...
	//bDisplayProperties指示是否自动弹出摄像头属性页
	//nWidth和nHeight设置的摄像头的宽和高，如果摄像头不支持所设定的宽度和高度，则返回false
	bool OpenCamera(int nCamID, bool bDisplayProperties=true, int nWidth=320, int nHeight=240);

    bool OpenCamera(int nCamID, const int& foramt, const int& width, const int& height, const int& fps, FRAME_CB_FUNC f = NULL);


	//关闭摄像头，析构函数会自动调用这个函数
	void CloseCamera();

	//返回摄像头的数目
	//可以不用创建CCameraDS实例，采用int c=CCameraDS::CameraCount();得到结果。
	static int CameraCount(); 

	//根据摄像头的编号返回摄像头的名字
	//nCamID: 摄像头编号
	//sName: 用于存放摄像头名字的数组
	//nBufferSize: sName的大小
	//可以不用创建CCameraDS实例，采用CCameraDS::CameraName();得到结果。
	static int CCameraDS::CameraName(int nCamID, char* sName, int nBufferSize);

    static int CCameraDS::CameraInfo(int nCamID, char* vid, char* pid);

	//返回图像宽度
	int GetWidth(){return m_nWidth;} 

	//返回图像高度
	int GetHeight(){return m_nHeight;}

	//抓取一帧，返回的IplImage不可手动释放！
	//返回图像数据的为RGB模式的Top-down(第一个字节为左上角像素)，即IplImage::origin=0(IPL_ORIGIN_TL)
	
};

#endif 

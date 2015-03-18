#pragma "once"
#include "stdafx.h"

#ifdef TOUCHPLUSDLL_EXPORTS
#define TOUCHPLUSDLL_API __declspec(dllexport) 
#else
#define TOUCHPLUSDLL_API __declspec(dllimport) 
#endif
#define VERBOSE 1

#define SIDE_LEFT	0
#define SIDE_RIGHT	1
#define SIDE_BOTH	2



#define MJPEG 1
#define UNCOMPRESSED 0
typedef void(*FRAME_CB_FUNC)(BYTE * pBuffer, long lBufferSize);
namespace TouchFuncs
{
	// This class is exported from the MathFuncsDll.dll
	class MyTouchFuncs
	{
	public:
		static TOUCHPLUSDLL_API int		do_software_unlock();
		static TOUCHPLUSDLL_API int		isCameraPresent();
		static TOUCHPLUSDLL_API int		setExposureTime(int whichSide, float expTime);
		static TOUCHPLUSDLL_API float	getExposureTime(int whichSide);
		static TOUCHPLUSDLL_API	int		setGlobalGain(int whichSide, float gain);
		static TOUCHPLUSDLL_API float   getGlobalGain(int whichSide);
		static TOUCHPLUSDLL_API int		turnLEDsOn();
		static TOUCHPLUSDLL_API int		turnLEDsOff();
		static TOUCHPLUSDLL_API int		getAccelerometerValues(int *x, int *y, int *z);
		static TOUCHPLUSDLL_API int		setColorGains(int whichSide, float red, float green, float blue);
		static TOUCHPLUSDLL_API int		getColorGains(int whichSide, float *red, float *green, float * blue);
		static TOUCHPLUSDLL_API int		enableAutoExposure(int whichSide);
		static TOUCHPLUSDLL_API int		disableAutoExposure(int whichSide);
		static TOUCHPLUSDLL_API int		enableAutoWhiteBalance(int whichSide);
		static TOUCHPLUSDLL_API int		disableAutoWhiteBalance(int whichSide);
		static TOUCHPLUSDLL_API int     startVideoStream(int width, int height, int framerate, int format, FRAME_CB_FUNC f );

	};
}
# TouchPlus
<b>SDK for the Ractiv Touch Plus<b>

The subfolders of the SDK folder contain the SDKs for the Win32 and Linux/Mac OS. 

Each folder contains a sample program (a simple viewer) which uses the SDK. The source code for the SDK also included. 
In each case, the viewer uses basic libraries from OpenCV to display the video streams. 

<b>For windows</b>
The SDK uses DirectShow for the video streams. Camera control is based on DLLs provided by Etron for low level control (exposure time, global gain, color balance etc). 

<b>Compiling Instructions for Windows</b>
Thanks for Andrew Kirkovski for finding this issue:
The project was build in VS 2013 and needs the following change in winnt.h to successfully compile:

from:
<pre>
//
// Void
//

typedef void *PVOID;
typedef void * POINTER_64 PVOID64;
</pre>

to:
<pre>
//
// Void
//
#define POINTER_64 __ptr64
typedef void *PVOID;
typedef void * POINTER_64 PVOID64;
</pre>

<b>For OS X and Linux</b>
In this case, the library used to stream is libUVC (see https://github.com/ktossell/libuvc).
Because Etron did not provide a library for these architectures, the control codes were "discovered" using the USB packets in the Win32 enviroment. These USB transfers are replicated using libUSB-1.0 (see http://www.libusb.org/wiki/libusb-1.0)

for more information on the use of these libraries contact corey@ractiv.com<br>
code improvements/bug reports may also be sent to corey@ractiv.com


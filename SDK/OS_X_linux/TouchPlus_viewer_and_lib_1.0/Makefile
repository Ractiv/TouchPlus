all: viewer  TouchPlus

TouchPlus: TouchPlus.cpp TouchPlus.h
	g++ -I/usr/local/include/libusb-1.0 -c TouchPlus.cpp  

viewer: viewer.cpp TouchPlus.o
	g++ viewer.cpp   TouchPlus.o -I/usr/local/include/libusb-1.0 -o viewer `pkg-config opencv --libs` -L./ -luvc -lusb-1.0

clean:
	rm viewer TouchPlus.o

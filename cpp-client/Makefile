CC = g++
CFLAGS = -std=c++11 -pthread
INCDIR = -I/home/pi/gitwerk/socket.io-client-cpp/build/include/
LIBDIR = -L/home/pi/gitwerk/socket.io-client-cpp/build/lib/Release/
LIBS = -lsioclient -lboost_system -lboost_date_time -lboost_random -l bcm2835 `pkg-config --cflags --libs opencv`

all: main.o camera.o CV_SubPix.o
	$(CC) -o main main.o camera.o CV_SubPix.o $(CFLAGS) $(INCDIR) $(LIBDIR) $(LIBS) 

main.o: main.cpp
	$(CC) -c main.cpp

CV_SubPix.o: CV_SubPix.cpp
	$(CC) -c CV_SubPix.cpp

camera.o: camera.h camera.cpp
	$(CC) $(CFLAGS) $(INCDIR) $(LIBDIR) $(LIBS) -c camera.cpp

clean:
	rm *.o

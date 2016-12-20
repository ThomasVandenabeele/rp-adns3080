#include "camera.h"
#include <bcm2835.h>
#include <unistd.h>
#include <stdio.h>

#include <sio_client.h>
#include <sio_message.h>
#include <sio_socket.h>

#include <opencv2/opencv.hpp>

// basic file operations
#include <iostream>
#include <fstream>

#include "CV_SubPix.h"


using namespace sio;
using namespace std;
using namespace cv;

uint8_t frame[ADNS3080_PIXELS_X * ADNS3080_PIXELS_Y];
ofstream pixelfile;
sio::client h;

Mat oldFrame;
bool start = true;
double xOffset = 0;
double yOffset = 0;

int resizewidth=30;
int resizeheight=30;

struct MD
{
 int motion;
 signed char  dx, dy;
 int squal;
 int shutter;
 uint8_t max_pix;
};

void mousecam_reset()
{
	bcm2835_gpio_write(PIN_MOUSECAM_RESET,HIGH);
	bcm2835_delayMicroseconds(1000); //usleep(1000); // reset pulse >10us
	bcm2835_gpio_write(PIN_MOUSECAM_RESET,LOW);
	bcm2835_delayMicroseconds(35000);// usleep(35000); // 35ms from reset to functional
	printf("Mousecam reset\n");
}

int mousecam_read_reg(int reg)
{
	bcm2835_gpio_write(PIN_MOUSECAM_CS, LOW);	// Select ADNS chip

  	bcm2835_spi_transfer(reg);

  	//usleep(75);
	bcm2835_delayMicroseconds(75);

  	int ret = bcm2835_spi_transfer(0xff);
	//printf("Read from mousecam : %d \n", ret);

  	bcm2835_gpio_write(PIN_MOUSECAM_CS,HIGH);	// Deselect ADNS chip

  	//usleep(1);
	bcm2835_delayMicroseconds(1);
  	return ret;
}

void mousecam_write_reg(int reg, int val)
{
	bcm2835_gpio_write(PIN_MOUSECAM_CS, LOW);	// Select ADNS chip

	// Send address
  	bcm2835_spi_transfer(reg | 0x80);	// binary or (reg | 0x80) for making MSB 1 !
  	// Send data
  	bcm2835_spi_transfer(val);
  	//printf("Write %d to mousecam address %d \n", val, reg);

  	bcm2835_gpio_write(PIN_MOUSECAM_CS, HIGH);	// Deselect ADNS chip
  	//usleep(50);
	bcm2835_delayMicroseconds(50);
}

int mousecam_init()
{
  	bcm2835_gpio_fsel(PIN_MOUSECAM_RESET, 0b001);
  	bcm2835_gpio_fsel(PIN_MOUSECAM_CS, 0b001);

  	bcm2835_gpio_write(PIN_MOUSECAM_CS,HIGH);	// Deselect ADNS chip for default operation

  	mousecam_reset();

  	int pid = mousecam_read_reg(ADNS3080_PRODUCT_ID);
  	printf("Mousecam PID: %d \n", pid);

  	if(pid != ADNS3080_PRODUCT_ID_VAL)
    	return -1;

  	// turn on sensitive mode
  	mousecam_write_reg(ADNS3080_CONFIGURATION_BITS, 0x19);

  	return 0;
}

void mousecam_read_motion(struct MD *p)
{
	bcm2835_gpio_write(PIN_MOUSECAM_CS, LOW);	// Select ADNS chip

  	bcm2835_spi_transfer(ADNS3080_MOTION_BURST);

	bcm2835_delayMicroseconds(75);

  	p->motion = bcm2835_spi_transfer(0xff);
  	p->dx =  bcm2835_spi_transfer(0xff);
  	p->dy =  bcm2835_spi_transfer(0xff);
  	p->squal =  bcm2835_spi_transfer(0xff);
  	p->shutter =  bcm2835_spi_transfer(0xff)<<8;
  	p->shutter |=  bcm2835_spi_transfer(0xff);
  	p->max_pix =  bcm2835_spi_transfer(0xff);

  	bcm2835_gpio_write(PIN_MOUSECAM_CS, HIGH);
  	bcm2835_delayMicroseconds(5);
}

// pdata must point to an array of size ADNS3080_PIXELS_X x ADNS3080_PIXELS_Y
// you must call mousecam_reset() after this if you want to go back to normal operation
int mousecam_frame_capture(uint8_t *pdata)
{
  	mousecam_write_reg(ADNS3080_FRAME_CAPTURE,0x83);

    	bcm2835_gpio_write(PIN_MOUSECAM_CS, LOW);	// Select ADNS chip

  	bcm2835_spi_transfer(ADNS3080_PIXEL_BURST);
  	//usleep(50);
	bcm2835_delayMicroseconds(50);

	int pix;
	uint8_t started = 0;
	int count;
	int timeout = 0;
	int ret = 0;
	for(count = 0; count < ADNS3080_PIXELS_X * ADNS3080_PIXELS_Y; )
	{
    		pix = bcm2835_spi_transfer(0xff); // Get pixel value
    		//usleep(10);
		bcm2835_delayMicroseconds(10);
    		if(started==0)
    		{
    			if(pix & 0x40)	// Check if it is first pixel of frame
      				started = 1;
    			else
    			{
      				timeout++;
      				if(timeout==100)
      				{
      					printf("A timeout occurred reading the frame.\n");
        				ret = -1;
        				break;
      				}
    			}
    		}
    		if(started==1)
    		{
      			pdata[count++] = (pix & 0x3f)<<2; // scale to normal grayscale uint8_t range
    		}
	}

	bcm2835_gpio_write(PIN_MOUSECAM_CS,HIGH);	// Deselect ADNS chip
	//usleep(14);
	bcm2835_delayMicroseconds(14);

	return ret;
}

void setup()
{
	printf("Start setup\n");

  	if(!bcm2835_init())
  	{
    	printf("bcm2835_init failed. Are you running as root??\n");
    	return;
  	}

  	if (!bcm2835_spi_begin())
    	{
      		printf("bcm2835_spi_begin failed. Are you running as root??\n");
      		return;
    	}
  	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  	bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
  	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_512);
  	bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

  	if(mousecam_init()==-1)
  	{
		printf("Mousecam_init failed.\n");
    		while(1);
  	}
	//namedWindow("window", WINDOW_NORMAL);
	//resizeWindow("window", 600, 600);

	h.connect("http://127.0.0.1:8900");
	//namedWindow("Display Image", WINDOW_AUTOSIZE );
}

void loop()
{
/**
	MD md;
  	mousecam_read_motion(&md);

	signed char X = (signed char)md.dx;
	signed char Y = (signed char)md.dy;
	
			//cout << "x: " << xOffset << ", Y: " << yOffset << endl;
	cout << "dx: " << +X << ", dY: " << +Y << endl;
**/

	//delay(75);
	
	if(mousecam_frame_capture(frame)==0)
  	{
		/**
		// Rotate image anti clockwise
		uint8_t frameRot[900];
		for (int row = 0; row < ADNS3080_PIXELS_X; row++)
		{
    			for (int col = 0; col < ADNS3080_PIXELS_Y; col++)
    			{
        			int offset = ADNS3080_PIXELS_Y * row + col;
        			frameRot[offset] = frame[((ADNS3080_PIXELS_Y * col) + (ADNS3080_PIXELS_X - 1)) - row];
    			}
		}
		**/
		
		Mat image(30, 30, CV_8UC1, frame, Mat::AUTO_STEP);

		Point2f pt(30/2.0F, 30/2.0F);
		Mat r = getRotationMatrix2D(pt, 90, 1.0);
		
		Mat dst;
		warpAffine(image, dst, r, image.size());

		//getRectSubPix(dst, dst.size(), pt, dst, -3);
		Mat dst2;
		//resize(dst, dst2, Size( resizewidth, resizeheight), (0,0), (0, 0), INTER_LINEAR);  //Size（resizewidth，resizeheight）
		//cout << dst << endl;
		
		//imshow("window",dst2);
		//waitKey(0);		

		// Convert image to char array
		char buffer[900];
		for (int j = 0; j < resizewidth; j++) {
		    for (int i = 0; i < resizeheight; i++) {
		        char& uxy = dst.at<char>(j, i);
		        int color = (int) uxy;
		        buffer[j * resizewidth + i] = color;
	             }
		}
		

		//Track displacement
		if(!start){
			Rect smallFrame(10, 10, 10, 10);
			Mat croppedRef(oldFrame, smallFrame);

			Mat cropped;
			// Copy the data into new matrix
			croppedRef.copyTo(cropped);

			Mat result;
			/// Create the result matrix
  			int result_cols =  dst.cols - cropped.cols + 1;
  			int result_rows = dst.rows - cropped.rows + 1;

  			result.create( result_rows, result_cols, CV_8UC1 );


			matchTemplate( dst, cropped, result, 3);
			normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
			
			/// Localizing the best match with minMaxLoc
  			double minVal; double maxVal; Point minLoc; Point maxLoc; Point2d subPixLoc;
  			Point matchLoc;

  			minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );	
			double test = minMaxLocSubPix(&subPixLoc, result, &maxLoc, 1);  			

			//cout << "Match: " << test << ", " << maxLoc << ", " << subPixLoc << endl;
			yOffset = -(10-subPixLoc.y);
                	xOffset = -(10-subPixLoc.x);			
			
/**
			delay(50);			
			MD md;
        		mousecam_read_motion(&md);

		        signed char X = (signed char)md.dx;
        		signed char Y = (signed char)md.dy;
        			
 	
	//		int X = (int)md.dx;
  	//		int Y = (int)md.dy;
	
			cout << "x: " << xOffset << ", Y: " << yOffset << endl;
			cout << "dx: " << X-255 << ", dY: " << Y-255 << endl;
**/
		}
		else{
			start = false;
		}
		oldFrame = dst;
		
		

		// Send displacement over websocket to server
		//h.socket()->emit("xoffset", to_string(xOffset));
		//h.socket()->emit("yoffset", to_string(yOffset));
		
		// Send image over websocket to server
		message::list arguments(to_string(xOffset));
		arguments.push(to_string(yOffset));
		arguments.push(std::make_shared<std::string>(buffer,900));//3481));
		//h.socket()->emit("pixels", std::make_shared<std::string>(buffer,900));//3481));
		h.socket()->emit("frame", arguments);
		delay(1);

		/**
		int i,j,k;
    		//pixelfile.open("/home/pi/adns3080/pixels.txt");
		printf("S\n");
	    	for(i=0, k=0; i<ADNS3080_PIXELS_Y; i++) 
	    	{
			for(j=0; j<ADNS3080_PIXELS_X; j++, k++) 
      			{
       				//pixelfile << (int)frame[k];
				//pixelfile << endl;
				printf("%d-", frameRot[k]);
  			}
  			printf("\n");
      		}
		//pixelfile.close();**/
  	}
}

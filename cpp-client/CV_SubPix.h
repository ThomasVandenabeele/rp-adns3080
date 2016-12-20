/*


*/ 
#pragma once

//#include "stdafx.h"			//PCH


#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)


//CV_EXPORTS_W 
double  minMaxLocSubPix(CV_OUT cv::Point2d* SubPixLoc,
						cv::InputArray src,						
                           CV_IN_OUT cv::Point* LocIn,						   
						   CV_IN_OUT const int Method = 0
                           );

/*
MinMaxLocSubPix
Copyright 2014 by bill Arden (William Arden) For OpenCV library. released under BSD license

3/9/2014. First version.  tested only with template with sharp edges. resulting scan range is +/- one pixel.

4/27/2014. Disabled changing the center. We need to constrain the change of the center to points where the heat map is exactly the same.

*/


//#include "stdafx.h"

#include "CV_SubPix.h"


#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)


using namespace cv;

#ifndef FLT_EPSILON
	#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */
#endif

#ifndef _ASSERT
	#define _ASSERT
#endif


void SubPixFitParabola(cv::Point2d* Result ,  cv::Point2d& P1, cv::Point2d& P2, cv::Point2d& P3);


double  minMaxLocSubPix(CV_OUT cv::Point2d* SubPixLoc,
						cv::InputArray src,						
                           CV_IN_OUT cv::Point* LocIn,						   
						   CV_IN_OUT const int Method 
                           )
{/*

in
	src = Heat map. It must be single-channel 32-bit floating-point
	LocIn = X,y location you want to start with. This is normally  from MinMaxLoc, however you can explore other peaks.	
	Method = -1 to disable math for testing, 0 for the best we have, 1 for this parabola curve fit version
out
	SubPixLoc = location adjusted with improved precision.

notes:
 If you rescale the heat map after match template... you must also adjust LocIn to match.

  */

	// set default result in case we bail
	SubPixLoc->x = (float)LocIn->x;
	SubPixLoc->y = (float)LocIn->y;

	if (Method == -1) { // disable any changes for testing		
		return 0;
	};

	// At this time we don't have anything other than Parabola math so we can ignore "Method".

	{ // Parabola math	
		
		/*
			The values returned from MatchTemplate are not linear past the point where it just starts to drop. 
			The reason is that we can not assume that the template is the same on all sides. Imagine a sloped edge on one side and a sharp edge on the other.			

			We can also get several values at the top that are all the same since the source and the template are integers.

			We also have to protect against the situation where the source is a solid white or solid black. The result is a constant value heat map.
		*/
		Mat HeatMap = src.getMat();

		// pick some limiting values
		// It's not expected that the template peak values will span more than 1/16 of the source size. This also limits the processing time used when looking at a blank image.
		Size MaxScan; MaxScan.width = HeatMap.cols >> 4;  MaxScan.height = HeatMap.rows >> 4; 		
		

		Point ScanRectMin;  // I used two Points instead of a Rect to prevent having Rect compute right/left values in each loop below
		Point ScanRectMax; 
		ScanRectMin.x = LocIn->x - MaxScan.width; if (ScanRectMin.x  < 0) ScanRectMin.x = 0;
		ScanRectMin.y = LocIn->y - MaxScan.height; if (ScanRectMin.y  < 0) ScanRectMin.y = 0;		
		ScanRectMax.x = LocIn->x + MaxScan.width; if (ScanRectMax.x  >=  HeatMap.cols ) ScanRectMax.x = HeatMap.cols -1;
		ScanRectMax.y = LocIn->y + MaxScan.height; if (ScanRectMax.y  >=  HeatMap.rows ) ScanRectMax.y = HeatMap.rows -1;

		// were we are starting at
		const float FloatValueChange = FLT_EPSILON * 10.0f; // smallest change that we can do math on with some meaningful result.

		// scan to find area to use. this can get complicated since we may be given a point near any of the edges of the blob we want to use.		
		float SrcStartingPoint =  HeatMap.at<float>( LocIn->y , LocIn->x);

		Point Center = *LocIn;

		// results
		Point ScanRight;
		Point ScanLeft;
		Point ScanUp;
		Point ScanDown;

		//for (int rescan = 0; rescan < 2; ++rescan){

			ScanRight = Center;
			while (true){
				++ScanRight.x; // no point checking the passed location. so inc first
				if (ScanRight.x > ScanRectMax.x){
//					_ASSERT(0);
					return 1; // ran out of room to scan
				};
				float Val = HeatMap.at<float>(ScanRight.y, ScanRight.x) ;
				if (abs( Val -  SrcStartingPoint) > FloatValueChange){
					break;
				};
			};

			ScanLeft = Center;
			while (true){
				--ScanLeft.x; // no point checking the passed location. so inc first
				if (ScanLeft.x < ScanRectMin.x){
//					_ASSERT(0);
					return 1; // ran out of room to scan
				};
				if (abs(HeatMap.at<float>( ScanLeft.y, ScanLeft.x) -  SrcStartingPoint) > FloatValueChange){
					break;
				};
			};

			ScanUp = Center;
			while (true){
				++ScanUp.y; // assume G cords. The actual direction of Up in the image is not important since the math is symmetrical
				if (ScanUp.y > ScanRectMax.y){
//					_ASSERT(0);
					return 1; // ran out of room to scan
				};
				if (abs(HeatMap.at<float>( ScanUp.y, ScanUp.x) -  SrcStartingPoint) > FloatValueChange){
					break;
				};
			};

			ScanDown = Center;
			while (true){
				--ScanDown.y; // assume G cords. The actual direction of Up in the image is not important since the math is symmetrical
				if (ScanDown.y < ScanRectMin.y){
//					_ASSERT(0);
					return 1; // ran out of room to scan
				};
				if (abs(HeatMap.at<float>(ScanDown.y, ScanDown.x) -  SrcStartingPoint) > FloatValueChange){
					break;
				};
			};

			// At this point we have a good starting point on the blob area, but our initial scan may be stuck on one side so center and rescan once more

			//Center.x =  ((ScanRight.x - ScanLeft.x) >> 1) + ScanLeft.x;
			//Center.y =  ((ScanUp.y    - ScanDown.y) >> 1) + ScanDown.y;

			// did center change?
			//if ((Center.x == LocIn->x) && (Center.y == LocIn->y)) break; // done early

		//}; // for rescan

		// measure polarity if needed
		


		// At this point we have a center of a blob with some extents to use

		// for each axis we now do a triangulation math.


		// imagine the match numbers as height and the pixel numbers as horizontal.

		//B is highest, A and C are on the sides

		
		double ErrorVal = 0;

		{// X axis

			 Point2d A;
			 A.x = ScanLeft.x; // The pixel cords
			 A.y = HeatMap.at<float>(ScanLeft.y, ScanLeft.x); // the Heat map value
			 
			 Point2d B; // center
			 B.x = Center.x; // The pixel cords
			 B.y = HeatMap.at<float>( Center.y, Center.x); // the Heat map value

			 Point2d C;
			 C.x = ScanRight.x; // The pixel cords
			 C.y = HeatMap.at<float>(ScanRight.y, ScanRight.x); // the Heat map value
			 
			 Point2d Result;
			 SubPixFitParabola( &Result ,  A, B, C);
			 // we throw away the y and use the x
			 
			 // clip and set error
			 if (Result.x < ScanLeft.x){
				 _ASSERT(0);
				 Result.x = ScanLeft.x;
				 ErrorVal = 1;
			 };
			 if (Result.x > ScanRight.x){
				 _ASSERT(0);
				 Result.x = ScanRight.x;
				 ErrorVal = 1;
			 };
			 SubPixLoc->x = Result.x;
		}; // X axis



		{// Y axis

			// this time we swap x and y since the parabola is always found in the x
			 Point2d A;
			 A.x = ScanDown.y; // The pixel cords
			 A.y = HeatMap.at<float>( ScanDown.y ,ScanDown.x); // the Heat map value
			 
			 Point2d B; // center
			 B.x = Center.y; // The pixel cords
			 B.y = HeatMap.at<float>( Center.y, Center.x); // the Heat map value

			 Point2d C;
			 C.x = ScanUp.y; // The pixel cords
			 C.y = HeatMap.at<float>( ScanUp.y, ScanUp.x); // the Heat map value
			 
			 Point2d Result;
			 SubPixFitParabola( &Result ,  A, B, C);
			 // we throw away the y and use the x
			 Result.y = Result.x;

			 // clip and set error
			 if (Result.y < ScanDown.y){
				 _ASSERT(0);
				 Result.y = ScanDown.y;
				 ErrorVal = 1;
			 };
			 if (Result.y > ScanUp.y){
				 _ASSERT(0);
				 Result.y = ScanUp.y;
				 ErrorVal = 1;
			 };
			 SubPixLoc->y = Result.y;
		}; // X axis


		return ErrorVal;
		

	}; // Bill's Tilt math

	return 0;

};

// Parabolic fit
void SubPixFitParabola(cv::Point2d* Result ,  cv::Point2d& P1, cv::Point2d& P2, cv::Point2d& P3)
{/*
 Parabola fit and resulting peak

 The parabola is aligned along the X axis with the peak being in the Y.

in
	P1 = a point on one side
	P2 = the center point
	P3 = a point on the other side
out
	Result = the peak point in the center of the parabola
*/ 

	Result->x = P2.x; // default in case of an error
	Result->y = P2.y;
	
	
	/* from http://stackoverflow.com/questions/717762/how-to-calculate-the-vertex-of-a-parabola-given-three-points
		This is really just a simple linear algebra problem, so you can do the calculation symbolically. When you substitute in the x and y values of your three points, you'll get three linear equations in three unknowns.

		A x1^2 + B x1 + C = y1
		A x2^2 + B x2 + C = y2
		A x3^2 + B x3 + C = y3

		The straightforward way to solve this is to invert the matrix

		x1^2  x1  1
		x2^2  x2  1
		x3^2  x2  1

		and multiply it by the vector

		y1
		y2
		y3

		The result of this is... okay, not exactly all that simple ;-) I did it in Mathematica, and here are the formulas in pseudocode:
	*/

	double denom = (P1.x - P2.x) * (P1.x - P3.x) * (P2.x - P3.x); // can't be zero since X is from pixel locations.
	double A = (P3.x * (P2.y - P1.y) + P2.x * (P1.y - P3.y) + P1.x * (P3.y - P2.y)) / denom;
	double B = ((P3.x * P3.x) * (P1.y - P2.y) + (P2.x * P2.x) * (P3.y - P1.y) + (P1.x * P1.x) * (P2.y - P3.y)) / denom;
	double C = (P2.x * P3.x * (P2.x - P3.x) * P1.y + P3.x * P1.x * (P3.x - P1.x) * P2.y + P1.x * P2.x * (P1.x - P2.x) * P3.y) / denom;


	
	// y = A * x^2 + B * x + C 

	//now find the center

	double xv = -B / (2 * A);
	double yv = C - (B*B) / (4 * A);
	 

	Result->x = xv;
	Result->y = yv;
};





//
//  main.cpp
//  hw4
//
//  Created by Ousyoukan on 16/1/10.
//  Copyright (c) 2016年 Ousyoukan. All rights reserved.
//




// Calibrate from a list of images
// Given a view of a checkerboard on a plane, view that image and a 
// list of others frontal parallel to that plane

#include <cvaux.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

int n_boards = 0; //Will be set by input list
const int board_dt = 10;
int board_w=12;
int board_h=12;


void help() {
	printf("Calibration from disk. Call convention:\n\n"
		"  ch11_ex11_1_fromdisk board_w board_h image_list\n\n"
		"Where: board_{w,h} are the # of internal corners in the checkerboard\n"
		"       width (board_w) and height (board_h)\n"
		"       image_list is space separated list of path/filename of checkerboard\n"
		"       images\n\n"
		"Hit 'p' to pause/unpause, ESC to quit.\n\n");
	
	printf("Birds eye view\n\n"
		"  birdseye board_w board_h intrinsics_mat.xml distortion_mat.xml checker_image \n\n"
		"Where: board_{w,h}    are the # of internal corners in the checkerboard\n"
		"       intrinsic      intrinsic path/name of matrix from prior calibration\n"
		"       distortion     distortion path/name of matrix from pror calibration\n"
		"       checker_image  is the path/name of image of checkerboard on the plane \n"
		"                      Frontal view of this will be shown.\n\n"
		" ADJUST VIEW HEIGHT using keys 'u' up, 'd' down. ESC to quit.\n\n");

}



int main(int argc, char* argv[]) {

	//CvCapture* capture;// = cvCreateCameraCapture( 0 );
					   // assert( capture );

	/*if (argc != 4) {
		help();
		return -1;
	}*/
	//help();
	if (argc > 2)
	{
		board_w = atoi(argv[1]);
		board_h = atoi(argv[2]);
	}
	int board_n = board_w * board_h;
	CvSize board_sz = cvSize(board_w, board_h);
	FILE *fptr = fopen("/Users/Ousyoukan/Desktop/hw4/hw4/ch11_chessboards.txt", "r");
	if(argc>3)fptr = fopen(argv[3], "r");

	char names[2048];
	//COUNT THE NUMBER OF IMAGES:
	while (fscanf(fptr, "%s ", names) == 1) {
		n_boards++;
	}
	rewind(fptr);

	cvNamedWindow("Calibration");
	//ALLOCATE STORAGE
	CvMat* image_points = cvCreateMat(n_boards*board_n, 2, CV_32FC1);
	CvMat* object_points = cvCreateMat(n_boards*board_n, 3, CV_32FC1);
	CvMat* point_counts = cvCreateMat(n_boards, 1, CV_32SC1);

	///  CvMat * image_points	= cvCreateMat(1, n_boards*board_n, CV_32FC2);
	///  CvMat * object_points = cvCreateMat(1, n_boards*board_n, CV_32FC3);
	///  CvMat * point_counts  = cvCreateMat(1, n_boards, CV_32SC1);

	CvMat* intrinsic_matrix = cvCreateMat(3, 3, CV_32FC1);
	CvMat* distortion_coeffs = cvCreateMat(4, 1, CV_32FC1);


	IplImage* image = 0;// = cvQueryFrame( capture );
	IplImage* gray_image = 0; //for subpixel
	CvPoint2D32f* corners = new CvPoint2D32f[board_n];
	int corner_count;
	int successes = 0;
	int step;

	for (int frame = 0; frame<n_boards; frame++) {
		fscanf(fptr, "%s ", names);

		if (image) {
			cvReleaseImage(&image);
			image = 0;
		}
		image = cvLoadImage(names);
		if (gray_image == 0 && image) //We'll need this for subpixel accurate stuff
			gray_image = cvCreateImage(cvGetSize(image), 8, 1);

		if (!image)
			printf("null image\n");

		int found = cvFindChessboardCorners(
			image,
			board_sz,
			corners,
			&corner_count,
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS
			);

		//Get Subpixel accuracy on those corners
		cvCvtColor(image, gray_image, CV_BGR2GRAY);
		cvFindCornerSubPix(gray_image, corners, corner_count,
			cvSize(11, 11), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
		//Draw it

		cvDrawChessboardCorners(image, board_sz, corners, corner_count, found);
		cvShowImage("Calibration", image);

		// If we got a good board, add it to our data
		//
		if (corner_count == board_n) {
			step = successes*board_n;
			//	printf("Found = %d for %s\n",found,names);
			for (int i = step, j = 0; j<board_n; ++i, ++j) {
				///         CV_MAT_ELEM(*image_points, CvPoint2D32f,0,i) = cvPoint2D32f(corners[j].x,corners[j].y);
				///         CV_MAT_ELEM(*object_points,CvPoint3D32f,0,i) = cvPoint3D32f(j/board_w, j%board_w, 0.0f);
				CV_MAT_ELEM(*image_points, float, i, 0) = corners[j].x;
				CV_MAT_ELEM(*image_points, float, i, 1) = corners[j].y;
				CV_MAT_ELEM(*object_points, float, i, 0) = j / board_w;
				CV_MAT_ELEM(*object_points, float, i, 1) = j%board_w;
				CV_MAT_ELEM(*object_points, float, i, 2) = 0.0f;

			}
			//        CV_MAT_ELEM(*point_counts, int,0,successes) = board_n;
			CV_MAT_ELEM(*point_counts, int, successes, 0) = board_n;
			successes++;
		}

		//    if( successes == n_boards ) break;

		int c = cvWaitKey(20);
		if (c == 'p') {
			c = 0;
			while (c != 'p' && c != 27) {
				c = cvWaitKey(250);
			}
		}
		if (c == 27)
			return 0;
	}
	
	cvDestroyWindow("Calibration");
	printf("successes = %d, n_boards=%d\n", successes, n_boards);
	

	//ALLOCATE MATRICES ACCORDING TO HOW MANY IMAGES WE FOUND CHESSBOARDS ON
	///  CvMat* image_points2      = cvCreateMat(1,successes*board_n,CV_32FC2);
	///  CvMat* object_points2     = cvCreateMat(1,successes*board_n,CV_32FC3);
	///  CvMat* point_counts2      = cvCreateMat(1,successes,CV_32SC1);
	CvMat* object_points2 = cvCreateMat(successes*board_n, 3, CV_32FC1);
	CvMat* image_points2 = cvCreateMat(successes*board_n, 2, CV_32FC1);
	CvMat* point_counts2 = cvCreateMat(successes, 1, CV_32SC1);
	//TRANSFER THE POINTS INTO THE CORRECT SIZE MATRICES
	for (int i = 0; i<successes*board_n; ++i) {
		///      CV_MAT_ELEM(*image_points2, CvPoint2D32f,0,i)  = CV_MAT_ELEM(*image_points, CvPoint2D32f,0,i);
		///      CV_MAT_ELEM(*object_points2,CvPoint3D32f,0,i)  = CV_MAT_ELEM(*object_points,CvPoint3D32f,0,i);
		CV_MAT_ELEM(*image_points2, float, i, 0) = CV_MAT_ELEM(*image_points, float, i, 0);
		CV_MAT_ELEM(*image_points2, float, i, 1) = CV_MAT_ELEM(*image_points, float, i, 1);
		CV_MAT_ELEM(*object_points2, float, i, 0) = CV_MAT_ELEM(*object_points, float, i, 0);
		CV_MAT_ELEM(*object_points2, float, i, 1) = CV_MAT_ELEM(*object_points, float, i, 1);
		CV_MAT_ELEM(*object_points2, float, i, 2) = CV_MAT_ELEM(*object_points, float, i, 2);

	}
	for (int i = 0; i<successes; ++i) {
		///		CV_MAT_ELEM(*point_counts2,int,0, i) = CV_MAT_ELEM(*point_counts, int,0,i);
		CV_MAT_ELEM(*point_counts2, int, i, 0) = CV_MAT_ELEM(*point_counts, int, i, 0);
	}
	cvReleaseMat(&object_points);
	cvReleaseMat(&image_points);
	cvReleaseMat(&point_counts);

	// cvWaitKey();//Now we have to reallocate the matrices
	// return 0;
	// At this point we have all of the chessboard corners we need.
	//

	// Initialize the intrinsic matrix such that the two focal
	// lengths have a ratio of 1.0
	//
	CV_MAT_ELEM(*intrinsic_matrix, float, 0, 0) = 1.0f;
	CV_MAT_ELEM(*intrinsic_matrix, float, 1, 1) = 1.0f;
	printf("cvCalibrateCamera2\n");
	cvCalibrateCamera2(
		object_points2,
		image_points2,
		point_counts2,
		cvGetSize(image),
		intrinsic_matrix,
		distortion_coeffs,
		NULL,
		NULL,
		0//CV_CALIB_FIX_ASPECT_RATIO
		);
	// Save our work
	cvSave("/Users/Ousyoukan/Desktop/hw4/hw4/Intrinsics.xml", intrinsic_matrix);
	cvSave("/Users/Ousyoukan/Desktop/hw4/hw4/Distortion.xml", distortion_coeffs);
	// Load test
	CvMat *intrinsic = (CvMat*)cvLoad("/Users/Ousyoukan/Desktop/hw4/hw4/Intrinsics.xml");
	CvMat *distortion = (CvMat*)cvLoad("/Users/Ousyoukan/Desktop/hw4/hw4/Distortion.xml");

	/*
	// Build the undistort map which we will use for all 
	// subsequent frames.
	//
	IplImage* mapx = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
	IplImage* mapy = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
	printf("cvInitUndistortMap\n");
	cvInitUndistortMap(
		intrinsic,
		distortion,
		mapx,
		mapy
		);
	// Just run the camera to the screen, now only showing the undistorted
	// image.
	//
	rewind(fptr);
	cvNamedWindow("Undistort");
	printf("\n\nPress any key to step through the images, ESC to quit\n\n");
	while (fscanf(fptr, "%s ", names) == 1) {
		if (image) {
			cvReleaseImage(&image);
			image = 0;
		}
		image = cvLoadImage(names);
		IplImage *t = cvCloneImage(image);
		cvShowImage("Calibration", image);
		cvRemap(t, image, mapx, mapy);
		cvReleaseImage(&t);
		//	  cvUndistort2(
		cvShowImage("Undistort", image);
		if ((cvWaitKey() & 0x7F) == 27) break;
	}
	

	CvSize board_sz = cvSize(board_w, board_h);

	CvMat *intrinsic = (CvMat*)cvLoad("Intrinsics.xml");
	CvMat *distortion = (CvMat*)cvLoad("Distortion.xml");*/

	image = 0;
	gray_image = 0;

	image = cvLoadImage("/Users/Ousyoukan/Desktop/hw4/hw4/ch12_birdseye.jpg");
	if (argc > 4)
	{
		image = cvLoadImage(argv[4]);
	}
	if (image == 0) {
		printf("Error: Couldn't load %s\n", argv[4]);
		return -1;
	}
	gray_image = cvCreateImage(cvGetSize(image), 8, 1);
	cvCvtColor(image, gray_image, CV_BGR2GRAY);

	//UNDISTORT OUR IMAGE
	IplImage* mapx = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
	IplImage* mapy = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
	cvInitUndistortMap(
		intrinsic,
		distortion,
		mapx,
		mapy
		);
	IplImage *t = cvCloneImage(image);
	cvRemap(t, image, mapx, mapy);

	//GET THE CHECKERBOARD ON THE PLANE
	cvNamedWindow("Checkers");
	
	corners = new CvPoint2D32f[board_n];
	corner_count = 0;
	
	int found = cvFindChessboardCorners(
		image,
		board_sz,
		corners,
		&corner_count,
		CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS
		);
	if (!found) {
		printf("Couldn't aquire checkerboard on %s, only found %d of %d corners\n",
			argv[5], corner_count, board_n);
		return -1;
	}
	//Get Subpixel accuracy on those corners
	cvFindCornerSubPix(gray_image, corners, corner_count,
		cvSize(11, 11), cvSize(-1, -1),
		cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

	//GET THE IMAGE AND OBJECT POINTS:
	//Object points are at (r,c): (0,0), (board_w-1,0), (0,board_h-1), (board_w-1,board_h-1)
	//That means corners are at: corners[r*board_w + c]
	CvPoint2D32f objPts[4], imgPts[4];
	objPts[0].x = 0;         objPts[0].y = 0;
	objPts[1].x = board_w - 1; objPts[1].y = 0;
	objPts[2].x = 0;         objPts[2].y = board_h - 1;
	objPts[3].x = board_w - 1; objPts[3].y = board_h - 1;
	imgPts[0] = corners[0];
	imgPts[1] = corners[board_w - 1];
	imgPts[2] = corners[(board_h - 1)*board_w];
	imgPts[3] = corners[(board_h - 1)*board_w + board_w - 1];

	//DRAW THE POINTS in order: B,G,R,YELLOW
	cvCircle(image, cvPointFrom32f(imgPts[0]), 9, CV_RGB(0, 0, 255), 3);
	cvCircle(image, cvPointFrom32f(imgPts[1]), 9, CV_RGB(0, 255, 0), 3);
	cvCircle(image, cvPointFrom32f(imgPts[2]), 9, CV_RGB(255, 0, 0), 3);
	cvCircle(image, cvPointFrom32f(imgPts[3]), 9, CV_RGB(255, 255, 0), 3);

	//DRAW THE FOUND CHECKERBOARD
	cvDrawChessboardCorners(image, board_sz, corners, corner_count, found);
	cvShowImage("Checkers", image);

	cvWaitKey(0);
	cvDestroyWindow("Checkers");

	//FIND THE HOMOGRAPHY
	CvMat *H = cvCreateMat(3, 3, CV_32F);
	CvMat *H_invt = cvCreateMat(3, 3, CV_32F);
	cvGetPerspectiveTransform(objPts, imgPts, H);

	//LET THE USER ADJUST THE Z HEIGHT OF THE VIEW
	float Z = 25;
	int key = 0;
	IplImage *birds_image = cvCloneImage(image);
	cvNamedWindow("Birds_Eye");
	while (key != 27) {//escape key stops
		CV_MAT_ELEM(*H, float, 2, 2) = Z;
		//	   cvInvert(H,H_invt); //If you want to invert the homography directly
		//	   cvWarpPerspective(image,birds_image,H_invt,CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
		//USE HOMOGRAPHY TO REMAP THE VIEW
		cvWarpPerspective(image, birds_image, H,
			CV_INTER_LINEAR + CV_WARP_INVERSE_MAP + CV_WARP_FILL_OUTLIERS);
		cvShowImage("Birds_Eye", birds_image);
		key = cvWaitKey();
		if (key == 'u') Z += 0.5;
		if (key == 'd') Z -= 0.5;
	}

	//SHOW ROTATION AND TRANSLATION VECTORS
	image_points = cvCreateMat(4, 1, CV_32FC2);
	object_points = cvCreateMat(4, 1, CV_32FC3);
	
	for (int i = 0; i<4; ++i) {
		CV_MAT_ELEM(*image_points, CvPoint2D32f, i, 0) = imgPts[i];
		CV_MAT_ELEM(*object_points, CvPoint3D32f, i, 0) = cvPoint3D32f(objPts[i].x, objPts[i].y, 0);
	}

	CvMat *RotRodrigues = cvCreateMat(3, 1, CV_32F);
	CvMat *Rot = cvCreateMat(3, 3, CV_32F);
	CvMat *Trans = cvCreateMat(3, 1, CV_32F);
	cvFindExtrinsicCameraParams2(object_points, image_points,
		intrinsic, distortion,
		RotRodrigues, Trans);
	cvRodrigues2(RotRodrigues, Rot);

	//SAVE AND EXIT
	cvSave("/Users/Ousyoukan/Desktop/hw4/hw4/Rot.xml", Rot);
	cvSave("/Users/Ousyoukan/Desktop/hw4/hw4/Trans.xml", Trans);
	cvSave("/Users/Ousyoukan/Desktop/hw4/hw4/H.xml", H);
	cvInvert(H, H_invt);
	cvSave("/Users/Ousyoukan/Desktop/hw4/hw4/H_invt.xml", H_invt); //Bottom row of H invert is horizon line


	return 0;
}

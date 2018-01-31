//
//  main.cpp
//  hw1
//
//  Created by Ousyoukan on 15/12/7.
//  Copyright (c) 2015年 Ousyoukan. All rights reserved.
//
#include <iostream>
#include <string>
#include <opencv2/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
//using namespace cv;

int main(int argc,char *argv[])
{
    CvSize standard = {512,460};
    CvPoint monji={60,400};
    CvFont
    
    IplImage *first_t=0;
    
    
    first_t = cvCreateImage( standard, first->depth, first->nChannels);
    second_t = cvCreateImage( standard, second->depth, second->nChannels);
    third_t = cvCreateImage( standard, third->depth, third->nChannels);
    fourth_t = cvCreateImage( standard, fourth->depth, fourth->nChannels);
    fifth_t = cvCreateImage( standard, fifth->depth, fifth->nChannels);
    cvResize(first,first_t,CV_INTER_LINEAR);
    cvResize(second,second_t,CV_INTER_LINEAR);
    cvResize(third,third_t,CV_INTER_LINEAR);
    cvResize(fourth,fourth_t,CV_INTER_LINEAR);
    cvResize(fifth,fifth_t,CV_INTER_LINEAR);
    cvPutText(first_t,"3120102053-XIAOHAN WANG",monji, &font , CV_RGB(255, 251, 240));
    while(1)
    {
        frame = cvQueryFrame(capture); //获取一帧图片
        if(frame == NULL)
            break;
        
        IplImage *dst = cvCreateImage(standard, frame->depth, frame->nChannels);
        
        cvResize( frame, dst, CV_INTER_LINEAR );
        cvPutText(dst,"3120102053-XIAOHAN WANG",monji, &font , CV_RGB(255, 251, 240));
        cvShowImage( "test", dst ); //将其显示
        char key = cvWaitKey(30);
        if(key==27)
            break;
        
        cvWriteFrame(writer, dst);
        frame = NULL;
        cvReleaseImage(&dst);
    }
    
    
    
}


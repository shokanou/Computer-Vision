//
//  main2.cpp
//  hw2
//
//  Created by Ousyoukan on 15/12/17.
//  Copyright (c) 2015年 Ousyoukan. All rights reserved.
//

#include "main2.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
using namespace cv;


//主函数
int main(int argc, char* argv[])
{
    
    cv::Mat  image, image1 = cv::imread ("/Users/Ousyoukan/Desktop/test.png");
    //灰度变换
    cv::cvtColor (image1,image,CV_BGR2GRAY);
    
    
    // 经典的harris角点方法
    harris Harris;
    // 计算角点
    Harris.detect(image);
    //获得角点
    std::vector<cv::Point> pts;
    Harris.getCorners(pts,0.01);
    // 标记角点
    Harris.drawOnImage(image,pts);
    
    cv::namedWindow ("harris");
    cv::imshow ("harris",image);
    cv::waitKey (0);
    return 0;
}

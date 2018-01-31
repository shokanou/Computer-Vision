//
//  main.cpp
//  hw2
//
//  Created by Ousyoukan on 15/12/16.
//  Copyright (c) 2015年 Ousyoukan. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;
    
    typedef struct HARRISPARAMS
    {
        int gaussSize; //高斯窗口 aperture_size
        float gaussSigma; //高斯方差
        double threshold; //对角点量设定的阈值
        int maximumSize; //局部极大值抑制时的窗口大小
        
    }HARRISPARAMS,*PHARRISPARAMS;
    
    /*******************************
     *对源图像进行卷积运算
     *输入项
     *srcFloat    源图像
     *Ix          卷积的结果
     *dxTemplate  卷积模板
     *widthTemplate 模板的宽度
     *heightTemplate 模板的高度
     ********************************/
    void convolution(IplImage* srcFloat,IplImage* Ix,double* dxTemplate , int widthTemplate,int heightTemplate);
    
    /***************************
     *harris角点检测函数
     *输入项
     *srcIn   源图像
     *params  harris角点检测需要的参数
     *corners 存放harris角点坐标
     ****************************/
    void getHarrisPoints(IplImage* srcIn,PHARRISPARAMS params,CvSeq* corners,double k);
    
    //主函数
    int main(int argc, char* argv[])
    {
        
        //相关变量
        IplImage* src,*srcGray;
        CvMemStorage* mem = cvCreateMemStorage(0);
        CvSeq* harrisPoints;

        CvSeq* redge;
        CvSeq* rflat;
        HARRISPARAMS harrisParams;
        string path;//"/Users/Ousyoukan/Desktop/test3.png"
        const char* path1;
        cin>>path;
        path1 = path.c_str();
        
        src = cvLoadImage(path1,1);//源图像
        
        srcGray = cvCreateImage(cvGetSize(src),8,1);        // 灰度图像
        
        if(!src)
        {
            cout << " src is null";
            return 0;
        }
        
        cvCvtColor(src,srcGray,CV_BGR2GRAY);
        
        //harris角点保存的空间  角点坐标保存在一个序列中
        harrisPoints = cvCreateSeq(0,sizeof(CvSeq),sizeof(CvPoint),mem);
        int ksize;
        double k;
        cin>>k;
        cin>>ksize;
        
        
        //设置相关参数
        harrisParams.gaussSize = ksize;//3;   // 高斯窗口的大小
        harrisParams.gaussSigma = 0.8;
        harrisParams.threshold = 300000000;//0.01;//
        harrisParams.maximumSize = 21;
 //=0.04;

        
        
        //进行harris角点检测
        getHarrisPoints(srcGray,&harrisParams,harrisPoints,k);
        
        //获取每一个角点的坐标
        for(int x=0;x<harrisPoints->total;x++)
        {
            //获取第x个角点的坐标
            CvPoint* pt = (CvPoint*)cvGetSeqElem(harrisPoints,x);
            
            //以角点坐标为中心  绘制一个半径为5的圆
            cvCircle(src,*pt,2,cvScalar(255,0,255,0));
        }
        
        cvSaveImage("/Users/Ousyoukan/Desktop/dst.jpg",src);
        
        //显示图像
        cvNamedWindow("dst");
        cvShowImage("dst",src);
        cvWaitKey(0);
        
        //释放资源
        cvReleaseImage(&src);
        cvReleaseImage(&srcGray);
        cvReleaseMemStorage(&mem);
        return 0;
    }
    
    /***************************
     *harris角点检测函数
     *输入项
     *srcIn   源图像
     *params  harris角点检测需要的参数
     *corners 存放harris角点坐标
     ****************************/
    void getHarrisPoints(IplImage* srcIn,PHARRISPARAMS params,CvSeq* corners,double k)
    {
        int x,y;
        double lamda1,lamda2;
        //double big,small;
        
        
        IplImage* srcFloat;
        IplImage* ookina;
        IplImage* chisai;
        IplImage* rpic;
        srcFloat = cvCreateImage(cvGetSize(srcIn),32,1);
        ookina = cvCreateImage(cvGetSize(srcIn), 32, 1);
        //cvSaveImage("/Users/Ousyoukan/Desktop/ookina.jpg", ookina);
        chisai = cvCreateImage(cvGetSize(srcIn), 32, 1);
        rpic = cvCreateImage(cvGetSize(srcIn), 32, 1);
        
        
        cvConvertScale(srcIn,srcFloat);
        
        IplImage *Ix,*Iy,*IxIx,*IyIy,*IxIy,*A,*B,*C,*cornerness;
        double *gaussWindow = new double[sizeof(double)*params->gaussSize*params->gaussSize];
        //水平方向差分算子并求Ix  sobel算子
        double dxTemplate[9]={-1,0,1,
            -2,0,2,
            -1,0,1};
        
        //垂直方向差分算子并求Iy
        double dyTemplate[9]={1,2,1,
            0, 0, 0,
            -1, -2, -1};
        

        Ix = cvCreateImage(cvGetSize(srcFloat),32,1);
        Iy = cvCreateImage(cvGetSize(srcFloat),32,1);
        IxIx = cvCreateImage(cvGetSize(srcFloat),32,1);
        IyIy = cvCreateImage(cvGetSize(srcFloat),32,1);
        IxIy = cvCreateImage(cvGetSize(srcFloat),32,1);
        A = cvCreateImage(cvGetSize(srcFloat),32,1);
        B = cvCreateImage(cvGetSize(srcFloat),32,1);
        C = cvCreateImage(cvGetSize(srcFloat),32,1);
        cornerness = cvCreateImage(cvGetSize(srcFloat),32,1); //保存角点量
        
        //Sobel(srcIn,Ix,CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);
        //Sobel(srcIn, Ix, CV_16S, 1, 0);
        convolution(srcFloat,Ix,dxTemplate,3,3); //计算Ix
        convolution(srcFloat,Iy,dyTemplate,3,3); //计算Iy
        
        //计算Ix2、Iy2、IxIy
        for(y=0;y<srcFloat->height;y++)
        {
            for(x=0;x<srcFloat->width;x++)
            {
                float IxValue,IyValue;
                
                IxValue = cvGetReal2D(Ix,y,x);
                IyValue = cvGetReal2D(Iy,y,x);
                
                cvSetReal2D(IxIx,y,x,IxValue*IxValue);
                cvSetReal2D(IyIy,y,x,IyValue*IyValue);
                cvSetReal2D(IxIy,y,x,IxValue*IyValue);
                
                
            }
        }
        
        //计算高斯窗口
        for( y=0;y<params->gaussSize;y++)
        {
            for( x=0;x<params->gaussSize;x++)
            {
                
                float dis,weight;
                
                dis = (y-params->gaussSize/2)*(y-params->gaussSize/2)+(x-params->gaussSize/2)*(x-params->gaussSize/2);
                weight = exp(-dis/(2.0*params->gaussSigma));
                *(gaussWindow+y*params->gaussSize+x)=weight;
                
            }
        }
        
        convolution(IxIx,A,gaussWindow,params->gaussSize,params->gaussSize);//计算IxIx与高斯的卷积
        convolution(IyIy,B,gaussWindow,params->gaussSize,params->gaussSize);//计算IyIy与高斯的卷积
        convolution(IxIy,C,gaussWindow,params->gaussSize,params->gaussSize);//计算IxIy与高斯的卷积
        
        //计算角点量
        /*big = cvGetReal2D(A, 0, 0)-cvGetReal2D(C, 0, 0);
        small = cvGetReal2D(B ,0,0)-cvGetReal2D(C , 0, 0);
        big = max(big,small);
        small = min(big,small);*/
        for(y=0;y<srcFloat->height;y++)
        {
            for(x=0;x<srcFloat->width;x++)
            {
                double cornernessValue,Avalue,Bvalue,Cvalue;
                Avalue = cvGetReal2D(A,y,x);
                Bvalue = cvGetReal2D(B,y,x);
                Cvalue = cvGetReal2D(C,y,x);
                lamda1 = Avalue-Cvalue;
                lamda2 = Bvalue-Cvalue;
                /*if(max(lamda1,lamda2)>big)
                    big = max(lamda1,lamda2);
                if(min(lamda1,lamda2)<small);
                    small = min(lamda1,lamda2);*/
                
                //cout<<lamda1<<endl;
                //cout<<lamda2<<endl;
                
                //cornernessValue = (Avalue*Bvalue-Cvalue*Cvalue)/(Avalue+Bvalue);//value of R
                cornernessValue = lamda1*lamda2 - (lamda1+lamda2)*(lamda1+lamda2)*k;
                //cout<<cornernessValue<<endl;
                cvSetReal2D(cornerness,y,x,cornernessValue);
            }
            
        }
        
        
        //计算局部极大值 及 极大值是否大于阈值
        int beginY,endY,beginX,endX;
        int halfWinSize = params->maximumSize/2;
        
        beginY = halfWinSize;
        endY = cornerness->height - halfWinSize;
        
        beginX = halfWinSize;
        endX = cornerness->width - halfWinSize;
        
        for(y=beginY;y<endY;)
        {
            for(x=beginX;x<endX;)
            {
                //寻找局部极大值 及其位置信息
                float maxValue=0;
                int flag = 0 ;
                CvPoint maxLoc;
                maxLoc.x = -1;
                maxLoc.y = -1;
                
                //首先计算以点(x,y)位中心的maximumSize*maximumSize的窗口内部的局部极大值
                for(int winy=-halfWinSize;winy<=halfWinSize;winy++)
                {
                    for(int winx=-halfWinSize;winx<=halfWinSize;winx++)
                    {
                        float value ;
                        value = cvGetReal2D(cornerness,y+winy,x+winx);
                        
                        //计算该窗口内 最大值 保存到max 并保存其坐标到maxLoc
                        if(value>maxValue)
                        {
                            maxValue = value;
                            maxLoc.x = x+winx;
                            maxLoc.y = y+winy;
                            flag = 1;
                        }
                    }
                }
                
                
                //如果找到局部极大值 并且该值大于预先设定的阈值 则认为是角点 corner  |R| small --flat R<0 --edge
                if(flag==1 && maxValue>params->threshold)
                {
                    cvSeqPush(corners,&maxLoc);
                    
                }
                //if(flag==1&&abs(maxValue)<=params->threshold)
                    //cvSeqPush(rflat,&maxLoc);
                //else
                    //cvSeqPush(redge,&maxLoc);
                
                x = x+params->maximumSize;
                
            }
            
            y = y + params->maximumSize;
            
        }
        
        delete []gaussWindow;
        cvReleaseImage(&Ix);
        cvReleaseImage(&Iy);
        cvReleaseImage(&IxIx);
        cvReleaseImage(&IyIy);
        cvReleaseImage(&IxIy);
        cvReleaseImage(&A);
        cvReleaseImage(&B);
        cvReleaseImage(&C);
        cvReleaseImage(&cornerness);
        cvReleaseImage(&srcFloat);
    }
    /*******************************
     *对源图像进行卷积运算
     *输入项
     *srcFloat    源图像
     *Ix          卷积的结果
     *dxTemplate  卷积模板
     *widthTemplate 模板的宽度
     *heightTemplate 模板的高度
     ********************************/  
    void convolution(IplImage* srcFloat,IplImage* Ix,double* dxTemplate , int widthTemplate,int heightTemplate)  
    {  
        int x,y,beginY,endY,beginX,endX;  
        
        beginY = heightTemplate/2;  
        endY = srcFloat->height - heightTemplate/2;  
        
        beginX = widthTemplate/2;  
        endX = srcFloat->width - widthTemplate/2;  
        
        for(y=beginY;y<endY;y++)  
        {  
            for(x=beginX;x<endX;x++)  
            {  
                
                int i,j;  
                double curDx=0;  
                
                
                for(i=0;i<heightTemplate;i++)  
                {  
                    for(j=0;j<widthTemplate;j++)  
                    {  
                        curDx += cvGetReal2D(srcFloat,y+i-heightTemplate/2,x+j-widthTemplate/2)**(dxTemplate+i*widthTemplate+j);  
                    }  
                }         
                cvSetReal2D(Ix,y,x,curDx);  
                
            }  
            
        }  
    }
    


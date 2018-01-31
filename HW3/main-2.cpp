//
//  main.cpp
//  hw3
//
//  Created by Ousyoukan on 15/12/16.
//  Copyright (c) 2015年 Ousyoukan. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>



using namespace std;
using namespace cv;
//#pragma comment(lib,"opencv_stitching245.lib")

string filepath = "/Users/Ousyoukan/Documents/test_face/";
string fullfile = "/Users/Ousyoukan/Documents/test_face/BioID_0011.pgm";


double facewidth=100,faceheight=100,eyeslength=0.4,leyepos=0.3,reyepos = 0.7,leyeheight = 0.35;
//void GetFace(char *filepath);
double total = faceheight*facewidth;

const string eyepath = "/Users/Ousyoukan/Documents/test_face/BioID_0011.eye";
const string modelpath = "/Users/Ousyoukan/Documents/test_face/model";


string int2str(int i);


void load(string &filepath,Mat& eigen,Mat& A );

int main(int argc, const char** argv)
{
    //const char* compfilepath;
    Mat src, face;
    int lx,ly,rx,ry;
    string temp_num;
    
    
    string testfilepath;
    Mat A,eigen;
    
    
    
    cin>>testfilepath>>modelpath;
    load(modelpath,eigen,A);
    
    
    Mat train(1,facewidth*faceheight,CV_32F);
    Mat sum(1,facewidth*faceheight,CV_32F);
    
    
    ifstream fin(eyepath);
    string str;
    getline(fin,str);
    fin>>lx>>ly>>rx>>ry;
    fin.close();
    
    Point2f eyesCenter;
    
    //旋转矩阵
    eyesCenter.x = (lx+rx)*0.5f;
    eyesCenter.y = (ly+ry)*0.5f;
    
    double angle = atan2(ry-ly,rx-lx) * 180.0/CV_PI + 180;
    double len = sqrt((ry-ly)*(ry-ly)+(rx-lx)*(rx-lx));
    double scale = eyeslength * facewidth /len;
    Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, scale);
    
    //仿射矩阵
    double ex = facewidth * 0.5f - eyesCenter.x;
    double ey = faceheight * leyepos - eyesCenter.y;
    rot_mat.at<double>(0,2) += ex;
    rot_mat.at<double>(1,2) += ey;
    Mat warped = Mat(faceheight,facewidth,CV_8U,Scalar(128));
    face = Mat(faceheight,facewidth,CV_8U,Scalar(128));
    
    //读取图片
    src = imread(fullfile,CV_LOAD_IMAGE_COLOR); //source img
    cvtColor(src, src, COLOR_BGR2GRAY);
    warpAffine(src, warped, rot_mat, warped.size());
    normalize(src, src, 0, 1, NORM_MINMAX);
    
    //均衡化
    //equalizeHist(warped, face);
    
    
    
    //cout<<face<<endl<<endl;
    //保存人脸
    //imwrite(filepath+"/face/test"+temp_num+".jpg", face);
    
    
    
    //Mat vec = Mat(faceheight*facewidth,1,CV_8U,Scalar(128));
    //vec = face.reshape(0,1);
    
    for(int j = 0;j<faceheight;j++){
        for(int k = 0;k<facewidth;k++){
            train.at<float>(j*facewidth+k) = warped.at<unsigned char>(j,k)/255.0;
        }
    }
    
    
    
    //特征
    Mat feature = eigen * train.t();
    Mat rec_vec = eigen.t() * feature;
    
    Mat rec(faceheight, facewidth, CV_32F, 0.0f);
    for( int i = 0; i < faceheight; i++ )
    {
        for( int j = 0; j < facewidth; j++ )
        {
            rec.at<float>(i,j) = rec_vec.at<float>(i*facewidth+j);
        }
    }
    normalize(rec, rec, 0, 1, NORM_MINMAX);
    
    
    int minpos = -1;
    double minsum = DBL_MAX;
    
    for(int i = 0; i < eigen.rows; i++)
    {
        double sum = 0;
        for( int j = 0; j < total; j++ )
        {
            sum += ( eigen.at<float>(i,j)*255 - rec_vec.at<float>(j)/255 ) * ( eigen.at<float>(i,j)*255 - rec_vec.at<float>(j)/255 );
        }
        
        
        if( sum < minsum )
        {
            minsum = sum;
            minpos = i;
        }
        sum /= eigen.rows;
        sum = sqrt(sum);
        
    }
    
    Mat showface(faceheight, facewidth*3, CV_32F, 0.0f);
    for( int i = 0; i < faceheight; i++ )
    {
        for( int j1 = 0; j1 < facewidth; j1++ )
        {
            showface.at<float>(i,j1) = rec.at<float>(i,j1);
        }
        for( int j2 = facewidth; j2< facewidth*2; j2++)
        {
            showface.at<float>(i,j2) = (rec.at<float>(i,j2-facewidth) + face.at<unsigned char>(i,j2-facewidth)/255.0)/2;
        }
        for( int j3 = facewidth*2; j3< facewidth*3; j3++)
        {
            showface.at<float>(i,j3) = face.at<unsigned char>(i,j3-facewidth*2)/255.0;
        }
    }
    
    //    normalize(rec, rec, 0, 255, NORM_MINMAX);
    cout << "The nearest face is at the No." << minpos << " evector" << endl;
    cout << "Min sum:" << minsum << endl;
    imshow("Result", showface);
    cvWaitKey(-1);
    
    return 0;
}


string int2str(int i)
{
    stringstream temp;
    string temp_num;
    temp<<i;
    temp_num = temp.str();
    
    return temp_num;
    
}

void load(string &filepath, Mat& eigen, Mat& A)
{
    FileStorage fs(filepath,FileStorage::READ);
    fs["eigen"] >> eigen;
    fs["A"] >> A;
    fs.release();
}
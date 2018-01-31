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
#include <opencv2/stitching/stitcher.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>



using namespace std;
using namespace cv;
//#pragma comment(lib,"opencv_stitching245.lib")

string filepath = "/Users/Ousyoukan/Documents/test_face/";
string fullfile;

double pct;
//string testfilepath;





string eyepath;
int i=0;
int K1 = 1520;

const double facewidth=100,faceheight=100,eyeslength=0.4,leyepos=0.3,reyepos = 0.7,leyeheight = 0.35;
//void GetFace(char *filepath);
double total = faceheight*facewidth;
string int2str(int i);
string testfilepath = "/Users/Ousyoukan/Documents/test_face/BioID_0001.pgm";
string testeyepath = "/Users/Ousyoukan/Documents/test_face/BioID_0001.eye";


void save(string &filepath,Mat& eigen,Mat& A );

int main(int argc, const char** argv)
{
    //const char* compfilepath;
    Mat src, face;
    int lx,ly,rx,ry;
    string temp_num;
    int K;

    string modelfilepath;
    string modelpath;
    
    
    cin>>pct>>modelpath;
    double tempk = pct*K1;
    K = static_cast<int>(tempk);
    
    modelfilepath = filepath+modelpath;
    //K = int(K1*pct);
    
    Mat train(K,facewidth*faceheight,CV_32F);
    Mat sum(1,facewidth*faceheight,CV_32F);
    
    for(i=1;i<=K;i++){
        if(i<10){
            
            temp_num = int2str(i);
            fullfile = filepath+"BioID_000"+temp_num+".pgm";
            eyepath = filepath+"BioID_000"+temp_num+".eye";
            //compfilepath = fullfile.c_str();
        }
        else if(i>=10&&i<100){
            temp_num = int2str(i);
            fullfile = filepath+"BioID_00"+temp_num+".pgm";
            //compfilepath = fullfile.c_str();
            eyepath = filepath+"BioID_00"+temp_num+".eye";
        }
        else if(i>=100&&i<1000){
            temp_num = int2str(i);
            fullfile = filepath+"BioID_0"+temp_num+".pgm";
            //compfilepath = fullfile.c_str();
            eyepath = filepath+"BioID_0"+temp_num+".eye";
        }
        else{
            temp_num = int2str(i);
            fullfile = filepath+"BioID_"+temp_num+".pgm";
            //compfilepath = fullfile.c_str();
            eyepath = filepath+"BioID_"+temp_num+".eye";
        }
        //cout<<fullfile<<endl;
        
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
        
        //均衡化
        equalizeHist(warped, face);
        
        //直方图拉伸
        double maxValue = 0, minValue = 0;
        minMaxLoc(face, &minValue, &maxValue);
        int mini = 0;
        for ( ; mini < 256; mini++ )
        {
            if(face.at<float>(mini) > (int)minValue)
                break;
        }
        int maxi = 255;
        for ( ; maxi >= 0 ; maxi-- )
        {
            if(face.at<float>(maxi) > (int)minValue)
                break;
        }
        
        Mat lookup(1, 256, CV_8U);
        for (int i = 0;i < 256; ++i)
        {
            if(i < mini) {
                lookup.at<uchar>(i) = 0;
            }
            else if(i > maxi) {
                lookup.at<uchar>(i) = 255;
            }
            else
                lookup.at<uchar>(i) =static_cast<uchar>((i - mini) * 255 / (maxi - mini) + 0.5);
        }
        Mat stretched;
        LUT(face,lookup,stretched);
        
        //cout<<face<<endl<<endl;
        //保存人脸
        imwrite(filepath+"/face/test"+temp_num+".jpg", face);
        

    
        //Mat vec = Mat(faceheight*facewidth,1,CV_8U,Scalar(128));
        //vec = face.reshape(0,1);
        
        for(int j = 0;j<faceheight;j++){
            for(int k = 0;k<facewidth;k++){
                train.at<float>(i-1,j*facewidth+k) = face.at<unsigned char>(j,k)/255.0;
            }
        }
        
    
        
    }
    
    //train = train.t();
    
    
    for(i=0;i<K;i++){
        Mat tmp_mat = train.row(i);
        sum += tmp_mat;
    }
    
    Mat average = sum.clone();
    average /= K;
    //cout<<average<<endl;
    Mat average_re(faceheight,facewidth,CV_32F);
    average_re = average.reshape(0,faceheight);
    
    //Mat conv_re(total*total,total*total,CV_32F);
    
    /*for(int j=0;j<1;j++){
            Mat face_tmp = train.row(j).reshape(0,faceheight);
        //cout<<face_tmp<<endl;
        Mat face_tmp2(total,total,CV_32F);
        
        
            face_tmp2 = (face_tmp-average_re).t()*(face_tmp-average_re);
        cout<<face_tmp2<<endl;
        //cout<<face_tmp2<<endl;
        }
    
    conv_re /= K;*/
    
    
    Mat avrt(K,total,CV_32F);
    
   Mat conv_mat(total,total,CV_32F);
    for(int j=0;j<K;j++){
        for(int x = 0;x<total;x++){
            avrt.at<float>(j,x) = average.at<float>(x);
        }
    }
    
    conv_mat = (train-avrt)*(train-avrt).t();
    
    conv_mat /= K;
    //cout<<conv_mat<<endl;
    Mat eigenvalue,eigenvector;
    
    eigen(conv_mat,eigenvalue,eigenvector);
    
    //cout<<eigenvector<<endl;
    double alpha = 0.95;
    int k;
    
    double eigenvaluesum =0;
    for(i =0;i<K;i++){
        eigenvaluesum += eigenvalue.at<float>(i);
    }
    
    double eigenvaluetest = 0;
    for(i = 0;i<K;i++)
    {
        eigenvaluetest += eigenvalue.at<float>(i);
        if(eigenvaluetest>=eigenvaluesum*alpha){
            k = i;
            break;
        }
    }
    
    cout<<k<<endl;
    
    //矩阵A和转置矩阵AT
    
    Mat A(K,k,CV_32F);
    
    for(i = 0;i<k;i++){
        for(int j=0;j<K;j++)
            A.at<float>(j,i) = eigenvector.at<float>(i,j);
    }
    //cout<<A<<endl;
    cout<<"-------"<<endl;
    //cout<<eigenvalue<<endl;
    Mat AT = A.t();
    
    Mat eigen = AT*train;
    
    for(int j=0;j<k;j++)
    {
        normalize(eigen.row(j), eigen.row(j),0,1,NORM_MINMAX);
    }
    
    //int rows = int(faceheight);
    
    //vector<Mat> imgs;
    //bool try_use_gpu = false;
    
    Mat eigenface(faceheight*2,facewidth*5,CV_8U,0.0f);
    //Mat eigen1,eigen_re1,eigen_re2;
    for(int j=0;j<(k>10?10:k);j++){
        
        Mat eigen1 = eigen.row(j);
        Mat eigen_re1(faceheight,facewidth,CV_8U,0.0f);
        for(int x=0;x<facewidth;x++){
            for(int y=0;y<faceheight;y++){
                if(eigen1.at<float>(x*facewidth+y)<=0)
                    eigen_re1.at<unsigned char>(x,y)=0;
                else if(eigen1.at<float>(x*facewidth+y)>=1)
                    eigen_re1.at<unsigned char>(x,y)=255;
                else
                    eigen_re1.at<unsigned char>(x,y) = (eigen1.at<float>(x*facewidth+y)*255.0f);
                
                eigenface.at<unsigned char>((j/5)*faceheight+x,(j%5)*facewidth+y)=eigen_re1.at<unsigned char>(x,y);
            }
        }
    }
    

    save(modelfilepath,eigenface,A);
    
    
    //imwrite(filepath+"/face/test/wwww1.jpg", eigenface);
    namedWindow("test");
    imshow("test",eigenface);
    waitKey(0);
    
    
    //cout<<eigen_re1;
    
    //return 0;
    
}

string int2str(int i){
    stringstream temp;
    string temp_num;
    temp<<i;
    temp_num = temp.str();
    
    return temp_num;
    
}

void save(string& filepath,Mat& eigen,Mat& A ){
    FileStorage fs(filepath,FileStorage::WRITE);
    fs<<"eigen"<<eigen;
    fs<<"A"<<A;
    fs.release();
}







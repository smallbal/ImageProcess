#ifndef _CRACKDETECT_H_
#define _CRACKDETECT_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>


using namespace cv;
using namespace std;


// #define INITIAL_N 21	//local window的初始size为21
// #define INITIAL_M 41	//local window的M设为41
#define INITIAL_N 5	//local window的初始size为21
#define INITIAL_M 9	//local window的M设为41
#define TS 0.60	//Fc阈值
#define BLACK 0
#define WHITE 255

enum {
	PERCOLATION_DP = 0,
	PERCOLATION_DC,
	PERCOLATION_NOTTEST = 128, //测试
	PERCOLATION_CRACK = 0,	//测试
	PERCOLATION_BACKGROUND = WHITE
};

#define PIXEL_VALUE(IMG, X, Y) (uchar)(*((IMG).data + (IMG).step[0] * (X) + (IMG).step[1] * (Y)))




float Func_Fc(	int Count,int Cmax);
float Func_Iterator_T(	const vector<uchar> & Ip, float T,float w);
float Func_Iterator_W( const float Fc );
const vector<uchar> & Func_Max_Pixel_Value(const Mat & img , const vector<Point> & pixels);


const vector<Point> Func_Pixel_8_Neighborhood( const Mat & img, const Point & p, vector<Point> &vec_out);
const vector<Point> Func_Area_8_Neighborhood( const Mat & img_naive, vector<Point> circle, Mat & img_state, vector<Point> &vec_out);
void Func_Area_State_Change(Mat & img_state , const vector<Point> & vec , int percolation_tag);

bool Func_Is_Edge(const Mat & img, const Point & p);
bool Func_Iter_Spread(const Mat & img_naive, Mat & img_state , Point & focal_pixel , vector<Point> & Dc);

void Func_Percolation( const Mat & img_naive, Mat & img_state , Mat & img_result);


#endif
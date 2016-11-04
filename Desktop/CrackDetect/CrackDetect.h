#ifndef _CRACKDETECT_H_
#define _CRACKDETECT_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>


using namespace cv;
using namespace std;


#define INITAL_N 3	//N�ĳ�ʼֵ
#define INITAL_M 9	//M�ĳ�ʼֵ
#define TS 0.60	//Fc��ֵ
#define BLACK 0
#define WHITE 255

enum {
	PERCOLATION_NOTTEST = 0,
	PERCOLATION_DP,
	PERCOLATION_DC,
	PERCOLATION_BACKGROUND,
	PERCOLATION_CRACK
};

#define PIXEL_VALUE(IMG, X, Y) (uchar)(*((IMG).data + (IMG).step[0] * (X) + (IMG).step[1] * (Y)))




float Func_Fc(	int Count,int Cmax);
float Func_Iterator_T(	const vector<Point> & Ip, uchar T,float w);
const vector<uchar> & Func_Max_Pixel_Value(const Mat & img , const vector<Point> & pixels);


const vector<Point> Func_Pixel_8_Neighborhood( const Mat & img, const Point & p, vector<Point> &vec_out);
const vector<Point> Func_Area_8_Neighborhood( const Mat & img_naive, vector<Point> circle, Mat & img_state, vector<Point> &vec_out);

bool Func_Is_Edge(const Mat & img, const Point & p);
bool Func_Iter_Spread(const Mat & img_naive, Mat & img_state , Point & focal_pixel , vector<Point> & Dc);

#endif
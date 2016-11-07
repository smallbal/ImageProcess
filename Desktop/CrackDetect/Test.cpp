#include "CrackDetect.h"

using namespace std;
using namespace cv;

static void PrintPointVec(const vector<Point> & vec)
{
	for(size_t i=0 ; i<vec.size(); ++i)
		cout<<vec[i]<<endl;
}
static void Test8Nei(const Mat & img, int x, int y)
{
	Point p = Point(x, y);
	vector<Point> pit;
	//pit = Func_Pixel_8_Neighborhood(img, p, pit);
	Func_Pixel_8_Neighborhood(img, p, pit);
	cout<<"点("<<x<<", "<<y<<")的8-邻域为：\n";
	PrintPointVec(pit);
	cout<<endl;
}
static void Test8NeiArea(const Mat & img_naive,  Mat & img_state , const vector<Point>area)
{
	for(size_t i = 0 ; i< area.size(); ++i)
	{
		img_state.at<uchar>(area[i]) = PERCOLATION_DP;
	}
	cout<<"状态图片 = "<<endl<<" "<<img_state<<endl<<endl;
	vector<Point> neighbor_8;
	Func_Area_8_Neighborhood(img_naive , area , img_state , neighbor_8);
	PrintPointVec(neighbor_8);
}


int iter_count = 0;

int main(int argc, char **argv)
{

	Mat img_naive = imread("./timg.jpg");
	Mat img_gray;	//灰度图像
	cv::cvtColor(img_naive, img_gray, cv::COLOR_BGR2GRAY);
	Mat img_state;
	Mat img_result;

	/*Mat img_result(3,3,CV_8UC1);
	for(size_t i = 0 ; i < img_result.rows ; ++i)
		for(size_t j = 0 ; j < img_result.cols ; ++j)
			img_result.at<uchar>(i,j) = i*3+j;
	cout<<img_result<<endl;
	cout<<PIXEL_VALUE(img_result , 1 ,2)-0<<endl;
	cout<<img_result.at<uchar>(Point(1,2))-0<<endl;*/
	/*cout<<img_naive.at<uchar>(Point(0,0))<<endl;*/

	Func_Percolation(img_gray , img_state , img_result);
	//Mat result(naive.rows, naive.cols,CV_8UC1,Scalar::all(PERCOLATION_NOTTEST));
	//cout<<"pixel value = "<<result.at<uchar>(Point(0,0))+0<<endl;
	// Point focal_pixel = Point(0,0); //焦点像素从（0,0）开始推移
	//float T = gray_img.at<float>(focal_pixel); //T初始值设为焦点像素的亮度值
	








	//通过
	// Mat test(10,10,CV_8UC1,Scalar::all(255));
	// Mat result(10,10,CV_8UC1,Scalar::all(PERCOLATION_NOTTEST));
	// vector<Point> vec_in;
	// vec_in.push_back(Point(2,2));
	// vec_in.push_back(Point(2,1));
	// vec_in.push_back(Point(8,5));
	// Test8NeiArea(test , result , vec_in);
	// cout<<"test = "<<endl<<" "<<test<<endl;
	// cout<<"result = "<<endl<<" "<<result<<endl;
	
	//通过
	// Test8Nei(gray_img, 0, 0);
	//Test8Nei(gray_img, 0, gray_img.cols-1);
	//Test8Nei(gray_img, gray_img.rows-1, 0);
	//Test8Nei(gray_img, gray_img.rows-1 , gray_img.cols-1);
	//Test8Nei(gray_img, 10, 10);
	//Test8Nei(gray_img, 10, gray_img.cols-1);
	//Test8Nei(gray_img, gray_img.rows-1, 10);
	//Test8Nei(gray_img, 10, 0);



	char c = 0;
	while((c = waitKey(0))!=27);


	
	
	return 0;
}

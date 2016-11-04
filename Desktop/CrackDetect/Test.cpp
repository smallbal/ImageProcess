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
	cout<<"��("<<x<<", "<<y<<")��8-����Ϊ��\n";
	PrintPointVec(pit);
	cout<<endl;
}
static void Test8NeiArea(const Mat & img_naive,  Mat & img_state , const vector<Point>area)
{
	for(size_t i = 0 ; i< area.size(); ++i)
	{
		img_state.at<uchar>(area[i]) = PERCOLATION_DP;
	}
	cout<<"״̬ͼƬ = "<<endl<<" "<<img_state<<endl<<endl;
	vector<Point> neighbor_8;
	Func_Area_8_Neighborhood(img_naive , area , img_state , neighbor_8);
	PrintPointVec(neighbor_8);
}


int iter_count = 0;

int main(int argc, char **argv)
{

	Mat naive = imread("./timg.jpg");
	Mat gray_img;	//�Ҷ�ͼ��
	cv::cvtColor(naive, gray_img, cv::COLOR_BGR2GRAY);
	//imshow("show", naive);
	//imshow("gray_img",gray_img);
	//Mat result(naive.rows, naive.cols,CV_8UC1,Scalar::all(PERCOLATION_NOTTEST));
	//cout<<"pixel value = "<<result.at<uchar>(Point(0,0))+0<<endl;
	int N =INITIAL_N;
	Point focal_pixel = Point(0,0); //�������شӣ�0,0����ʼ����
	float T = gray_img.at<float>(focal_pixel); //T��ʼֵ��Ϊ�������ص�����ֵ



	//ͨ��
	// Mat test(10,10,CV_8UC1,Scalar::all(255));
	// Mat result(10,10,CV_8UC1,Scalar::all(PERCOLATION_NOTTEST));
	// vector<Point> vec_in;
	// vec_in.push_back(Point(2,2));
	// vec_in.push_back(Point(2,1));
	// vec_in.push_back(Point(8,5));
	// Test8NeiArea(test , result , vec_in);
	// cout<<"test = "<<endl<<" "<<test<<endl;
	// cout<<"result = "<<endl<<" "<<result<<endl;
	
	//ͨ��
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
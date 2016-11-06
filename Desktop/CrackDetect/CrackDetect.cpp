
#include <math.h> //abs()
#include <algorithm> /*find()*/
#include "CrackDetect.h"
//Opencv中的Point(x,y)对应坐标系的(col,row)


 static void Static_Func_Detect_Dc_Element(Mat & img_state , const vector<Point> & vec_in , vector<Point> & vec_out )
{
	for(size_t i = 0 ; i < vec_in.size() ; ++i)
	{
 		//cout<<"Pixel"<<vec_in[i]<<" PixValue = "<<PIXEL_VALUE(img_state, vec_in[i].y, vec_in[i].x)-0<<endl;
		if(PIXEL_VALUE(img_state, vec_in[i].y , vec_in[i].x) == PERCOLATION_NOTTEST)
		{
			vec_out.push_back(vec_in[i]);
			img_state.at<uchar>(vec_in[i]) = PERCOLATION_DC;
			//cout<<"now the value is "<<PIXEL_VALUE(img_state, vec_in[i].y, vec_in[i].x)-0<<endl;
		}
		// cout<<endl;
	}
}


const vector<Point>  Func_Pixel_8_Neighborhood( const Mat & img, const Point & p, vector<Point> &vec_out)
{
	/*
		x>0	&& y>0      |    x>0   | x>0 && y<rols-1
		------------------------------------------------
	    y>0             |   (x,y)    | y<rols-1
		-------------------------------------------------
		x<rows-1 && y>0 | x<rows-1 | x<rows-1 && y<rols-1
	*/
	if(p.x > 0)
	{
		vec_out.push_back(Point(p.x-1, p.y));
		if(p.y > 0)
		{
			vec_out.push_back( Point( p.x-1, p.y-1));
		}
		if(p.y< (img.rows -1) )
		{
			vec_out.push_back(Point(p.x-1, p.y+1));
		}
	}
	if(p.x < (img.cols -1))
	{
		vec_out.push_back(Point( p.x+1, p.y));
		if(p.y > 0)
			vec_out.push_back(Point(p.x+1, p.y-1));
		if(p.y < (img.rows - 1))
			vec_out.push_back(Point(p.x+1, p.y+1));
	}
	if(p.y>0)
		vec_out.push_back(Point(p.x, p.y-1));
	if(p.y < (img.rows - 1))
		vec_out.push_back(Point(p.x, p.y+1));
	return vec_out;
}

const vector<Point> Func_Area_8_Neighborhood( const Mat & img_naive, vector<Point> circle, Mat & img_state, vector<Point> & vec_out)
{
	vector<Point> neighbor;
	for (size_t i = 0 ; i < circle.size() ; ++i)
	{
		Func_Pixel_8_Neighborhood(img_naive, circle[i], neighbor);	
	}
	Static_Func_Detect_Dc_Element(img_state, neighbor, vec_out);
	/*for(size_t i = 0 ; i < vec_out.size() ; ++i)
	{
		img_state.at<uchar>(vec_out[i]) = PERCOLATION_DC;
	}*/
	return vec_out;
}

void Func_Area_State_Change(Mat & img_state , const vector<Point> & vec , int percolation_tag)
{
	for(size_t i = 0 ; i < vec.size() ; ++i)
	{
		img_state.at<uchar>(vec[i]) = percolation_tag;
	}
}



//Fc = (4* Count) / (PI * Cmax^2)
float Func_Fc(	int Count, //Dp中的像素个数
						int Cmax //Dp的最大长度
					 ){
	return (float)(4*Count/CV_PI/Cmax/Cmax);
}


const vector<uchar> & Func_Max_Pixel_Value(const Mat & img , const vector<Point> & pixels)
{
	vector<uchar> *pixel_value = new vector<uchar>;
	for(size_t i = 0 ; i < pixels.size() ; ++i)
	{
		pixel_value->push_back(PIXEL_VALUE(img , pixels[i].x , pixels[i].y));
	}
	return *pixel_value;
}

//T = max{ max[ I(p) ] , T } + w, 
float Func_Iterator_T(const vector<uchar> & Ip, //I(p)
									   float T,	//上次的亮度阈值T
									   float w)	//加速因子
{
	float I_max = Ip[0];
	for(size_t i = 1; i<Ip.size(); ++i)
	{
		if(Ip[i]>I_max)
			I_max = Ip[i];
	}
	if(I_max < T)
		I_max = T;
	return I_max+w;
}

//w' = Fc * w
float Func_Iterator_W(const float Fc)
{
	return Fc*255;
}


void Func_Percolation( const Mat & img_gray , Mat & img_state , Mat & img_result)
{
	img_state = Mat(img_gray.rows , img_gray.cols , CV_8UC1 , Scalar(PERCOLATION_NOTTEST));
	Mat img_gray_with_border;
	//增加上下左右各宽为M的边框，便于percolation
	copyMakeBorder(img_gray , img_gray_with_border , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT ,Scalar(WHITE));
	copyMakeBorder(img_state,img_state , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT , Scalar(PERCOLATION_BACKGROUND));
	
	int N = INITIAL_N;
	int M = INITIAL_M;
	int x =0 ;	//不加边框灰度图的列col
	int y =0;	//不加边框灰度图的行row
	Point focal_pixel = Point(x+INITIAL_M-1 ,y+INITIAL_M-1); //img_gray_with_border上对应的坐标
	
	Mat local_window_gray;
	Mat local_window_state;
	vector<Point> Dp;	//Dp记录的是local_window的坐标
	vector<uchar> Ip;	//Ip记录的是Dp的亮度值
	vector<Point> Dc;	//Dc记录的是local_window的坐标
	float Fc = 0;
	float T = img_gray.at<uchar>(focal_pixel);
	float w = 0;	//加速因子
	
	int iter_count = 0;
	// while(x < img_gray.cols && y < img_gray.rows)
	// {
		if(x == 0 || x == img_gray.cols || y ==0 || y == img_gray.rows)
		{
			N = INITIAL_N * 2 -1; 	//41
			M = INITIAL_M * 2 - 1 ;	//81
		}
		else
		{
			N = INITIAL_N;		//21
			M =INITIAL_M;		//41
		}
		focal_pixel = Point(x+INITIAL_M-1 ,y+INITIAL_M-1);	//这是img_gray_with_border 上的坐标
		/*Mat::Mat(const Mat& m, const Rect& roi)
			{
				CV_Assert( m.dims <= 2 );
				...
				CV_Assert( 0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= m.cols &&
              0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= m.rows );
			}
		*/
		//cout<<"[x,y] "<<focal_pixel<<endl;
		local_window_gray = img_gray_with_border( Rect(focal_pixel.x-(M-1)/2 , focal_pixel.y -(M-1)/2 , M , M));
		local_window_state = img_state( Rect(focal_pixel.x-(M-1)/2 , focal_pixel.y -(M-1)/2 , M , M));
		//local_window_gray = Mat(img_gray_with_border , Rect(focal_pixel.x-(M-1)/2 , focal_pixel.y -(M-1)/2 , M , M));
		Dp.clear();
		while(iter_count <= 4)
		{
		++iter_count;
		cout<<"iter_count = "<<iter_count<<endl;
		Ip.clear();
		//第1次迭代：将focal_pixel加入Dp
		if(iter_count == 1)
		{
			Dp.push_back(Point((M-1)/2,(M-1)/2));
		}
		for(size_t i = 0 ; i < Dp.size() ; ++i)
		{
			//判断Dp是否触及边界N
			if(abs(Dp[i].x - (N-1)/2 >= (N-1)/2) || abs(Dp[i].y - (N-1)/2) >= (N-1)/2)
				break;//触及边界则退出当前focal_pixel的渗透循环
		}
		//将Dp的亮度Ip求出
		for(size_t i = 0 ; i< Dp.size() ; ++i)
		{
			cout<<"Dp["<<i<<"] = "<<Dp[i]<<endl;
			Ip.push_back(PIXEL_VALUE(local_window_gray, Dp[i].x , Dp[i].y));
		}
		cout<<endl;
		T = Func_Iterator_T(Ip , T , w); 
		//求出此轮Dp的8-邻域Dc，并在img_state中将相应像素的状态标记为PERCOLATION_DC
		Func_Area_8_Neighborhood(local_window_gray , Dp , local_window_state , Dc);
		Func_Area_State_Change(img_state , Dp , PERCOLATION_DP);	//将Dp对应像素在img_state上标记为PERCOLATION_DP

		//判断Dc中的亮度，小于T的对应Dc中的像素被归入Dp,并改变Dc的相应标记为Dp
		int Dc2Dp = 0;
		uchar Darkest_Value = WHITE;
		Point Darkest_Point = Dc[0];
		int i =0;
		for(vector<Point>::iterator iter = Dc.begin() ; iter != Dc.end() ; ++iter)
		{
			++i;
			cout<<"Dc["<<i<<"] = "<<*iter<<endl;
			//记录Dc中亮度最低的点坐标及其亮度值
			if(Darkest_Value > local_window_gray.at<uchar>(*iter))
			{
				Darkest_Value = local_window_gray.at<uchar>(*iter);
				Darkest_Point = *iter;
			}
			if(local_window_gray.at<uchar>(*iter) < T)
			{
				Dp.push_back(*iter);
				local_window_state.at<uchar>(*iter) = PERCOLATION_DP;	//改变img_state窗口中由Dc转入Dp的像素的状态值
				Dc.erase(iter);
				++Dc2Dp;
			}
		}
		//若Dc中没有像素的零度小于T,则将Dc中亮度最小的像素归入Dp
		if(Dc2Dp == 0)
		{
			Dp.push_back(Darkest_Point);
			local_window_state.at<uchar>(Darkest_Point) = PERCOLATION_DP;
			Dc.erase(find(Dc.begin() , Dc.end() , Darkest_Point));
		}
		for(size_t i = 0 ; i<Dc.size() ; ++i)
		{
			local_window_state.at<uchar>(Dc[i]) = PERCOLATION_NOTTEST;
		}
		for(size_t i = 0; i<Dp.size(); ++i)
		{
			cout<<"Dp["<<i<<"] = "<<Dp[i]<<endl;
		}
		cout<<endl;
		}
}

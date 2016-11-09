
#include <math.h> //abs()
#include <algorithm> /*find()*/
// #include<Windows.h> //高精度时间计算
#include <time.h> //clock() , CLOCKS_PER_SEC
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


//编写后找到findContours()函数
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
	// return 2.0*Count/CV_PI/Cmax/Cmax;
	return 4.0*Count/CV_PI/Cmax/Cmax;
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
	img_result = Mat(img_gray.rows, img_gray.cols , CV_8UC1 , Scalar(WHITE));
	Mat img_gray_with_border;
	//增加上下左右各宽为M的边框，便于percolation
	// copyMakeBorder(img_gray , img_gray_with_border , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT ,Scalar(WHITE));
	// copyMakeBorder(img_state,img_state , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT , Scalar(PERCOLATION_BACKGROUND));
	
	copyMakeBorder(img_gray , img_gray_with_border , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_REPLICATE);
	copyMakeBorder(img_state,img_state , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT , Scalar(PERCOLATION_NOTTEST));
	
	float Ts = TS;
	int N = INITIAL_N;
	int M = INITIAL_M;
	int x =50 ;	//不加边框灰度图的列col
	int y =0;	//不加边框灰度图的行row
	int pixel_number = 0; //从0到img_gray.size() - 1
	Point focal_pixel = Point(x+INITIAL_M-1 ,y+INITIAL_M-1); //img_gray_with_border上对应的坐标
	
	Mat local_window_gray;
	Mat local_window_state;	//标注状态的窗口，状态有：NOTTEST , CRACK , BACKGROUND
	Mat local_window_D_state; //标注Dc和Dp的局部窗口,状态有: Dc , Dp ，NOTTEST
	vector<Point> Dp;	//Dp记录的是local_window的坐标
	vector<uchar> Ip;	//Ip记录的是Dp的亮度值
	vector<Point> Dc;	//Dc记录的是local_window的坐标
	float Fc = 0.0;
	float T = img_gray.at<uchar>(x,y);
	float w = 0;	//加速因子
	
	int iter_count = 0;
	while(x < img_gray.cols && y < img_gray.rows)
	// while(pixel_number < 2)
	{
		if(x == 0 || x == img_gray.cols-1 || y ==0 || y == img_gray.rows-1)
		{
			N = INITIAL_N * 2 -1; 	
			M = INITIAL_M * 2 - 1 ;	
		}
		else
		{
			N = INITIAL_N;		
			M =INITIAL_M;		
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
		// cout<<"[x,y] "<<focal_pixel<<endl;
		local_window_gray = img_gray_with_border( Rect(focal_pixel.x-(M-1)/2 , focal_pixel.y -(M-1)/2 , M , M)); 
		local_window_state = img_state( Rect(focal_pixel.x-(M-1)/2 , focal_pixel.y -(M-1)/2 , M , M));
		local_window_D_state = Mat(M, M, CV_8UC1, Scalar(PERCOLATION_NOTTEST));
		
		Dp.clear();
		iter_count = 0;
		
		//local_window_state.at<uchar>(Point((M-1)/2-1 , (M-1)/2)) = PERCOLATION_BACKGROUND;
		do	//对focal_pixel的渗透
		{
			++iter_count;
			// cout<<"iter_count = "<<iter_count<<endl;
			Dc.clear();
			Ip.clear();
			//第1次迭代：将focal_pixel加入Dp
			if(iter_count == 1)
			{
					Dp.push_back(Point((M-1)/2,(M-1)/2));
					local_window_D_state.at<uchar>(Point((M-1)/2,(M-1)/2)) = PERCOLATION_DP;
				
			}
			
			//将Dp的亮度Ip求出
			for(size_t i = 0 ; i< Dp.size() ; ++i)
			{
				// Ip.push_back(PIXEL_VALUE(local_window_gray, Dp[i].y , Dp[i].x));
				Ip.push_back(local_window_gray.at<uchar>(Dp[i]));
			}
			
			//第2次迭代前判断上次的Dp中是否包含背景点
			if(iter_count == 2)
			{
				vector<Point>::iterator iter = Dp.begin();
				for(; iter != Dp.end() ; ++iter)
				{
					if(local_window_state.at<uchar>(*iter) == PERCOLATION_BACKGROUND)
						break;
				}
				if(iter != Dp.end())
				{
					// cout<<"jump"<<endl;
					Fc = 1.0;
					break;
				}
			}

			T = Func_Iterator_T(Ip , T , w); 
			//将Dp对应像素在local_window_D_state上标记为PERCOLATION_DP
			Func_Area_State_Change(local_window_D_state , Dp , PERCOLATION_DP);	
			//求出此轮Dp的8-邻域Dc，并在img_state和local_window_state中将相应像素的状态标记为PERCOLATION_DC
			Func_Area_8_Neighborhood(local_window_gray , Dp , local_window_D_state , Dc);

			//判断Dc中的亮度，小于T的对应Dc中的像素被归入Dp,并改变Dc的相应标记为Dp
			int Dc2Dp = 0;
			uchar Darkest_Value = WHITE;	//Dc中亮度最低值
			vector<Point> Darkest_Points;	//Dc中亮度最低值的点的坐标
			for(vector<Point>::iterator iter = Dc.begin() ; iter != Dc.end(); )
			{
				//剔除Dc中不在N*N区域中的点
				if((*iter).x < (M-N)/2 || (*iter).x > (M+N-2)/2 || (*iter).y < (M-N)/2 || (*iter).y > (M+N-2)/2)
				{
					local_window_D_state.at<uchar>(*iter) = PERCOLATION_NOTTEST;
					iter = Dc.erase(iter);
				}
				else
				{
					++iter;
				}
			}
			// cout<<"M = "<<M<<" N = "<<N<<endl;
			// cout<<"iter_count "<<iter_count<<endl;
			// for(size_t i = 0 ; i < Dc.size() ;++i)
			// {
				// cout<<" Dc["<<i<<"] = "<<Dc[i]<<" ";
			// }
			// cout<<endl;
			for(vector<Point>::iterator iter = Dc.begin() ; iter != Dc.end() ;)
			{
				//记录Dc中亮度最低值
				uchar temp_lightness = local_window_gray.at<uchar>(*iter);
				if(Darkest_Value > temp_lightness)
				{
					//更新Dc中亮度最小值，并清空坐标vector
					Darkest_Value = temp_lightness;
					Darkest_Points.clear();
				}
				if(Darkest_Value == temp_lightness)
				{
					Darkest_Points.push_back(*iter);
				}
				//若有满足亮度低于T的像素点，将其转入Dp
				if(local_window_gray.at<uchar>(*iter) < T)
				{
					Dp.push_back(*iter);
					//改变img_state窗口中由Dc转入Dp的像素的状态值
					local_window_D_state.at<uchar>(*iter) = PERCOLATION_DP;	
					//.erase(iter)后，iter就会被析构从而成为一个“野指针”，则下一次迭代的++iter就会出错
					iter = Dc.erase(iter);
					++Dc2Dp;
				}
				else
				{
					++iter;
				}
			}
			//若Dc中没有像素的零度小于T,则将Dc中亮度为最小值的所有像素归入Dp
			if(Dc2Dp == 0)
			{
				Dp.insert(Dp.end(), Darkest_Points.begin(), Darkest_Points.end());
			}
			//将此轮没有被归入Dp的Dc重新标记为NOTTEST
			for(size_t i = 0 ; i<Dc.size() ; ++i)
			{
				local_window_D_state.at<uchar>(Dc[i]) = PERCOLATION_NOTTEST;
			}
			//计算Dp的外接矩形(直立矩形)
			Rect rect = boundingRect(Dp);
			// cout<<"Dp外接up-right 矩形为["<<rect.x<<", "<<rect.y<<"] and width="<<rect.width<<" height = "<<rect.height<<endl;
			//Dp是否触界判断	
			if((rect.width >= N) || (rect.height >= N))
			{
				//外接矩形宽度到达N
				Fc = Func_Fc(Dp.size(),max(rect.width , rect.height));
				// cout<<"focal_pixel = "<<focal_pixel<<endl;
				// cout<<Dp.size()<<" "<<max(rect.width , rect.height)<<" "<<Fc<<endl;
				if(Fc > Ts || N >= M) //Fc > Ts为terminate加速操作
				{
					// cout<<"end"<<endl;
					break;
				}
				else
				{
					N += 2;
				}
			}
			// cout<<endl<<endl;
		}while(true);
		//将Fc*255作为亮度值放到img_result中作为亮度
		
		//cout<<Fc<<endl;
		if(Fc < Ts)
		{
			//若Fc小于Ts，则将focal_pixel对应的Dp中所有的pixel在img_state中标记为CRACK
			img_state.at<uchar>(focal_pixel) = PERCOLATION_CRACK;
			//focal_pixel为Crack，则将此次percolation的Dp所在img_state全部标为CRACK
			// for(size_t i = 0 ; i <Dp.size() ; ++i)
			// {
				// if(local_window_state.at<uchar>(Dp[i]) == PERCOLATION_NOTTEST)
				// {
					// if((x==0 && Dp[i].x>=(M-1)/2) || (x==img_gray.cols-1 && Dp[i].x<=(M-1)/2)
						// || (y==0 && Dp[i].y>=(M-1)/2) || (y==img_gray.rows-1 && Dp[i].y<=(M-1)/2))
					// {
						// local_window_state.at<uchar>(Dp[i]) = PERCOLATION_CRACK;
						// img_result.at<uchar>(Dp[i].y+y-(M-1)/2,Dp[i].x+x-(M-1)/2) = (uchar)(Fc*WHITE);
					// }
				// }
			// }
			// cout<<"Crack"<<endl;
		}
		else
		{
			Fc = 1;
			img_state.at<uchar>(focal_pixel) = PERCOLATION_BACKGROUND;
			// cout<<"Background"<<endl;
		}
		img_result.at<uchar>(Point(x,y)) = (uchar)(Fc*WHITE);
		// img_result.at<uchar>(Point(x,y)) = (uchar)(Fc*WHITE);
		 // Mat roi = img_gray(Rect(x-(M-1)/2, y-(M-1)/2,M,M));
		// cout<<"roi = "<<endl<<" "<<roi<<endl;
		// cout<<"roi = "<<endl<<" "<<roi<<endl;
		
		// cout<<"local_gray = "<<endl<<" "<<local_window_gray<<endl;
		// cout<<"D_state = "<<endl<<" "<<local_window_D_state<<endl;
		
		// cout<<"["<<x<<", "<<y<<"]点的state = "<<img_state.at<uchar>(focal_pixel)+0<<" 结果亮度="
			// <<img_result.at<uchar>(Point(x,y))+0<<endl;
		
		// imshow("local_D", local_window_D_state);
		// cout<<local_window_D_state<<endl;
		do
		{
			++pixel_number;
			y = pixel_number/img_gray.cols;
			x = pixel_number%img_gray.cols;
		}while((img_state.at<uchar>(y+INITIAL_M-1, x+INITIAL_M-1) != PERCOLATION_NOTTEST) && (x < img_gray.cols) && (y < img_gray.rows));
		
	}
}

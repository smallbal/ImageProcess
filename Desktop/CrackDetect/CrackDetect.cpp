
#include <math.h> //abs()
#include <algorithm> /*find()*/
// #include<Windows.h> //¸ß¾«¶ÈÊ±¼ä¼ÆËã
#include <time.h> //clock() , CLOCKS_PER_SEC
#include "CrackDetect.h"
//OpencvÖĞµÄPoint(x,y)¶ÔÓ¦×ø±êÏµµÄ(col,row)


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


//±àĞ´ºóÕÒµ½findContours()º¯Êı
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
float Func_Fc(	int Count, //DpÖĞµÄÏñËØ¸öÊı
						int Cmax //DpµÄ×î´ó³¤¶È
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
									   float T,	//ÉÏ´ÎµÄÁÁ¶ÈãĞÖµT
									   float w)	//¼ÓËÙÒò×Ó
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
	//Ôö¼ÓÉÏÏÂ×óÓÒ¸÷¿íÎªMµÄ±ß¿ò£¬±ãÓÚpercolation
	copyMakeBorder(img_gray , img_gray_with_border , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT ,Scalar(WHITE));
	copyMakeBorder(img_state,img_state , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT , Scalar(PERCOLATION_BACKGROUND));
	
	double Ts = TS;
	int N = INITIAL_N;
	int M = INITIAL_M;
	int x =0 ;	//²»¼Ó±ß¿ò»Ò¶ÈÍ¼µÄÁĞcol
	int y =0;	//²»¼Ó±ß¿ò»Ò¶ÈÍ¼µÄĞĞrow
	int pixel_number = 0; //´Ó0µ½img_gray.size() - 1
	Point focal_pixel = Point(x+INITIAL_M-1 ,y+INITIAL_M-1); //img_gray_with_borderÉÏ¶ÔÓ¦µÄ×ø±ê
	
	Mat local_window_gray;
	Mat local_window_state;	//±ê×¢×´Ì¬µÄ´°¿Ú£¬×´Ì¬ÓĞ£ºNOTTEST , CRACK , BACKGROUND
	Mat local_window_D_state; //±ê×¢DcºÍDpµÄ¾Ö²¿´°¿Ú,×´Ì¬ÓĞ: Dc , Dp £¬NOTTEST
	vector<Point> Dp;	//Dp¼ÇÂ¼µÄÊÇlocal_windowµÄ×ø±ê
	vector<uchar> Ip;	//Ip¼ÇÂ¼µÄÊÇDpµÄÁÁ¶ÈÖµ
	vector<Point> Dc;	//Dc¼ÇÂ¼µÄÊÇlocal_windowµÄ×ø±ê
	float Fc = 0.0;
	float T = img_gray.at<uchar>(focal_pixel);
	float w = 0;	//¼ÓËÙÒò×Ó
	
	int iter_count = 0;
	while(x < img_gray.cols && y < img_gray.rows)
	{
		if(x == 0 || x == img_gray.cols-1 || y ==0 || y == img_gray.rows-1)
		{
			N = INITIAL_N * 2 -1; 	//41
			M = INITIAL_M * 2 - 1 ;	//81
		}
		else
		{
			N = INITIAL_N;		//21
			M =INITIAL_M;		//41
		}
		focal_pixel = Point(x+INITIAL_M-1 ,y+INITIAL_M-1);	//ÕâÊÇimg_gray_with_border ÉÏµÄ×ø±ê
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
		local_window_D_state = Mat(M, M, CV_8UC1, Scalar(PERCOLATION_NOTTEST));
		//local_window_gray = Mat(img_gray_with_border , Rect(focal_pixel.x-(M-1)/2 , focal_pixel.y -(M-1)/2 , M , M));
		Dp.clear();
		iter_count = 0;
		do
		{
			++iter_count;
			// cout<<"iter_count = "<<iter_count<<endl;
			Dc.clear();
			Ip.clear();
			//µÚ1´Îµü´ú£º½«focal_pixel¼ÓÈëDp
			if(iter_count == 1)
			{
				Dp.push_back(Point((M-1)/2,(M-1)/2));
				local_window_D_state.at<uchar>(Point((M-1)/2,(M-1)/2)) = PERCOLATION_DP;
			}
			
			//½«DpµÄÁÁ¶ÈIpÇó³ö
			for(size_t i = 0 ; i< Dp.size() ; ++i)
			{
				// cout<<"Dp["<<i<<"] = "<<Dp[i]<<endl;
				Ip.push_back(PIXEL_VALUE(local_window_gray, Dp[i].x , Dp[i].y));
			}
			
			//µÚ2´Îµü´úÇ°ÅĞ¶ÏÉÏ´ÎµÄDpÖĞÊÇ·ñ°üº¬±³¾°µã
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
					Fc = 1.0;
					break;
				}
			}
			// cout<<endl;
			T = Func_Iterator_T(Ip , T , w); 
			//Çó³ö´ËÂÖDpµÄ8-ÁÚÓòDc£¬²¢ÔÚimg_stateºÍlocal_window_stateÖĞ½«ÏàÓ¦ÏñËØµÄ×´Ì¬±ê¼ÇÎªPERCOLATION_DC
			Func_Area_8_Neighborhood(local_window_gray , Dp , local_window_D_state , Dc);
			//½«Dp¶ÔÓ¦ÏñËØÔÚlocal_window_D_stateÉÏ±ê¼ÇÎªPERCOLATION_DP
			Func_Area_State_Change(local_window_D_state , Dp , PERCOLATION_DP);	

			//ÅĞ¶ÏDcÖĞµÄÁÁ¶È£¬Ğ¡ÓÚTµÄ¶ÔÓ¦DcÖĞµÄÏñËØ±»¹éÈëDp,²¢¸Ä±äDcµÄÏàÓ¦±ê¼ÇÎªDp
			int Dc2Dp = 0;
			uchar Darkest_Value = WHITE;
			Point Darkest_Point = Dc[0];
			int i =0;	//for debug
			for(vector<Point>::iterator iter = Dc.begin() ; iter != Dc.end() ;)
			{
				++i;
				// cout<<"Dc["<<i<<"] = "<<*iter<<endl;
				//¼ÇÂ¼DcÖĞÁÁ¶È×îµÍµÄµã×ø±ê¼°ÆäÁÁ¶ÈÖµ
				if(Darkest_Value > local_window_gray.at<uchar>(*iter))
				{
					Darkest_Value = local_window_gray.at<uchar>(*iter);
					Darkest_Point = *iter;
				}
				if(local_window_gray.at<uchar>(*iter) < T)
				{
					Dp.push_back(*iter);
					//¸Ä±äimg_state´°¿ÚÖĞÓÉDc×ªÈëDpµÄÏñËØµÄ×´Ì¬Öµ
					local_window_D_state.at<uchar>(*iter) = PERCOLATION_DP;	
					//.erase(iter)ºó£¬iter¾Í»á±»Îö¹¹´Ó¶ø³ÉÎªÒ»¸ö¡°Ò°Ö¸Õë¡±£¬ÔòÏÂÒ»´Îµü´úµÄ++iter¾Í»á³ö´í
					iter = Dc.erase(iter);
					++Dc2Dp;
				}
				else
				{
					++iter;
				}
			}
			//ÈôDcÖĞÃ»ÓĞÏñËØµÄÁã¶ÈĞ¡ÓÚT,Ôò½«DcÖĞÁÁ¶È×îĞ¡µÄÏñËØ¹éÈëDp
			if(Dc2Dp == 0)
			{
				Dp.push_back(Darkest_Point);
				local_window_D_state.at<uchar>(Darkest_Point) = PERCOLATION_DP;
				//ÒòÎª¾­¹ıDc2Dp==0µÄÅĞ¶Ï£¬ÔòDcÖĞÒ»¶¨ÓĞDarkest_Point£¬ËùÒÔfind()²»»á·´»ÚDcµÄend£¬
				//Òò¶øÒ²¾Í²»»á½«endÉ¾³ıÒıÆğÂé·³
				Dc.erase(find(Dc.begin() , Dc.end() , Darkest_Point));
			}
			//½«´ËÂÖÃ»ÓĞ±»¹éÈëDpµÄDcÖØĞÂ±ê¼ÇÎªNOTTEST
			for(size_t i = 0 ; i<Dc.size() ; ++i)
			{
				local_window_D_state.at<uchar>(Dc[i]) = PERCOLATION_NOTTEST;
			}
			// cout<<endl;
			// for(size_t i = 0; i<Dp.size(); ++i)
			// {
				// cout<<"Dp["<<i<<"] = "<<Dp[i]<<endl;
			// }
			//¼ÆËãDpµÄÍâ½Ó¾ØĞÎ(Ö±Á¢¾ØĞÎ)
			Rect rect = boundingRect(Dp);
			// cout<<"DpÍâ½Óup-right ¾ØĞÎÎª["<<rect.x<<", "<<rect.y<<"] and width="<<rect.width<<" height = "<<rect.height<<endl;
			//DpÊÇ·ñ´¥½çÅĞ¶Ï
			if(rect.width == N || rect.height == N)
			{
				//Íâ½Ó¾ØĞÎ¿í¶Èµ½´ïN
				Fc = Func_Fc(Dp.size(),max(rect.width , rect.height));
				if(Fc > Ts || N >= M) //Fc > TsÎªterminate¼ÓËÙ²Ù×÷
				{
					break;
				}
				else
				{
					N += 2;
				}
			}
			// cout<<endl<<endl;
		}while(iter_count <=4);
		//½«Fc*255×÷ÎªÁÁ¶ÈÖµ·Åµ½img_resultÖĞ×÷ÎªÁÁ¶È
		img_result.at<uchar>(Point(x,y)) = (uchar)(Fc*255);
		if(Fc < Ts)
		{
			//ÈôFcĞ¡ÓÚTs£¬Ôò½«focal_pixel¶ÔÓ¦µÄDpÖĞËùÓĞµÄpixel¾ùÔÚimg_stateÖĞ±ê¼ÇÎªCRACK
			img_state.at<uchar>(focal_pixel) = PERCOLATION_CRACK;
		}
		else
		{
			//ÈôFc´óÓÚTs£¬Ôò½«focal_pixel¶ÔÓ¦µÄDpÖĞËùÓĞµÄpixel¾ùÔÚimg_stateÖĞ±ê¼ÇÎªBACKGROUND
			img_state.at<uchar>(focal_pixel) = PERCOLATION_BACKGROUND;
		}
		cout<<"["<<x<<", "<<y<<"]µãµÄstate = "<<img_state.at<uchar>(focal_pixel)+0<<" ½á¹ûÁÁ¶È="
			<<img_result.at<uchar>(Point(x,y))+0<<endl;
		++pixel_number;
		y = pixel_number/img_gray.cols;
		x = pixel_number%img_gray.cols;
	}
}

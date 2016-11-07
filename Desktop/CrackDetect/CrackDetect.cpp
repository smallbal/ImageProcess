
#include <math.h> //abs()
#include <algorithm> /*find()*/
// #include<Windows.h> //�߾���ʱ�����
#include <time.h> //clock() , CLOCKS_PER_SEC
#include "CrackDetect.h"
//Opencv�е�Point(x,y)��Ӧ����ϵ��(col,row)


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


//��д���ҵ�findContours()����
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
float Func_Fc(	int Count, //Dp�е����ظ���
						int Cmax //Dp����󳤶�
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
									   float T,	//�ϴε�������ֵT
									   float w)	//��������
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
	//�����������Ҹ���ΪM�ı߿򣬱���percolation
	copyMakeBorder(img_gray , img_gray_with_border , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT ,Scalar(WHITE));
	copyMakeBorder(img_state,img_state , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , INITIAL_M-1 , BORDER_CONSTANT , Scalar(PERCOLATION_BACKGROUND));
	
	double Ts = TS;
	int N = INITIAL_N;
	int M = INITIAL_M;
	int x =0 ;	//���ӱ߿�Ҷ�ͼ����col
	int y =0;	//���ӱ߿�Ҷ�ͼ����row
	int pixel_number = 0; //��0��img_gray.size() - 1
	Point focal_pixel = Point(x+INITIAL_M-1 ,y+INITIAL_M-1); //img_gray_with_border�϶�Ӧ������
	
	Mat local_window_gray;
	Mat local_window_state;	//��ע״̬�Ĵ��ڣ�״̬�У�NOTTEST , CRACK , BACKGROUND
	Mat local_window_D_state; //��עDc��Dp�ľֲ�����,״̬��: Dc , Dp ��NOTTEST
	vector<Point> Dp;	//Dp��¼����local_window������
	vector<uchar> Ip;	//Ip��¼����Dp������ֵ
	vector<Point> Dc;	//Dc��¼����local_window������
	float Fc = 0.0;
	float T = img_gray.at<uchar>(focal_pixel);
	float w = 0;	//��������
	
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
		focal_pixel = Point(x+INITIAL_M-1 ,y+INITIAL_M-1);	//����img_gray_with_border �ϵ�����
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
			//��1�ε�������focal_pixel����Dp
			if(iter_count == 1)
			{
				Dp.push_back(Point((M-1)/2,(M-1)/2));
				local_window_D_state.at<uchar>(Point((M-1)/2,(M-1)/2)) = PERCOLATION_DP;
			}
			
			//��Dp������Ip���
			for(size_t i = 0 ; i< Dp.size() ; ++i)
			{
				// cout<<"Dp["<<i<<"] = "<<Dp[i]<<endl;
				Ip.push_back(PIXEL_VALUE(local_window_gray, Dp[i].x , Dp[i].y));
			}
			
			//��2�ε���ǰ�ж��ϴε�Dp���Ƿ����������
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
			//�������Dp��8-����Dc������img_state��local_window_state�н���Ӧ���ص�״̬���ΪPERCOLATION_DC
			Func_Area_8_Neighborhood(local_window_gray , Dp , local_window_D_state , Dc);
			//��Dp��Ӧ������local_window_D_state�ϱ��ΪPERCOLATION_DP
			Func_Area_State_Change(local_window_D_state , Dp , PERCOLATION_DP);	

			//�ж�Dc�е����ȣ�С��T�Ķ�ӦDc�е����ر�����Dp,���ı�Dc����Ӧ���ΪDp
			int Dc2Dp = 0;
			uchar Darkest_Value = WHITE;
			Point Darkest_Point = Dc[0];
			int i =0;	//for debug
			for(vector<Point>::iterator iter = Dc.begin() ; iter != Dc.end() ;)
			{
				++i;
				// cout<<"Dc["<<i<<"] = "<<*iter<<endl;
				//��¼Dc��������͵ĵ����꼰������ֵ
				if(Darkest_Value > local_window_gray.at<uchar>(*iter))
				{
					Darkest_Value = local_window_gray.at<uchar>(*iter);
					Darkest_Point = *iter;
				}
				if(local_window_gray.at<uchar>(*iter) < T)
				{
					Dp.push_back(*iter);
					//�ı�img_state��������Dcת��Dp�����ص�״ֵ̬
					local_window_D_state.at<uchar>(*iter) = PERCOLATION_DP;	
					//.erase(iter)��iter�ͻᱻ�����Ӷ���Ϊһ����Ұָ�롱������һ�ε�����++iter�ͻ����
					iter = Dc.erase(iter);
					++Dc2Dp;
				}
				else
				{
					++iter;
				}
			}
			//��Dc��û�����ص����С��T,��Dc��������С�����ع���Dp
			if(Dc2Dp == 0)
			{
				Dp.push_back(Darkest_Point);
				local_window_D_state.at<uchar>(Darkest_Point) = PERCOLATION_DP;
				//��Ϊ����Dc2Dp==0���жϣ���Dc��һ����Darkest_Point������find()���ᷴ��Dc��end��
				//���Ҳ�Ͳ��Ὣendɾ�������鷳
				Dc.erase(find(Dc.begin() , Dc.end() , Darkest_Point));
			}
			//������û�б�����Dp��Dc���±��ΪNOTTEST
			for(size_t i = 0 ; i<Dc.size() ; ++i)
			{
				local_window_D_state.at<uchar>(Dc[i]) = PERCOLATION_NOTTEST;
			}
			// cout<<endl;
			// for(size_t i = 0; i<Dp.size(); ++i)
			// {
				// cout<<"Dp["<<i<<"] = "<<Dp[i]<<endl;
			// }
			//����Dp����Ӿ���(ֱ������)
			Rect rect = boundingRect(Dp);
			// cout<<"Dp���up-right ����Ϊ["<<rect.x<<", "<<rect.y<<"] and width="<<rect.width<<" height = "<<rect.height<<endl;
			//Dp�Ƿ񴥽��ж�
			if(rect.width == N || rect.height == N)
			{
				//��Ӿ��ο�ȵ���N
				Fc = Func_Fc(Dp.size(),max(rect.width , rect.height));
				if(Fc > Ts || N >= M) //Fc > TsΪterminate���ٲ���
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
		//��Fc*255��Ϊ����ֵ�ŵ�img_result����Ϊ����
		img_result.at<uchar>(Point(x,y)) = (uchar)(Fc*255);
		if(Fc < Ts)
		{
			//��FcС��Ts����focal_pixel��Ӧ��Dp�����е�pixel����img_state�б��ΪCRACK
			img_state.at<uchar>(focal_pixel) = PERCOLATION_CRACK;
		}
		else
		{
			//��Fc����Ts����focal_pixel��Ӧ��Dp�����е�pixel����img_state�б��ΪBACKGROUND
			img_state.at<uchar>(focal_pixel) = PERCOLATION_BACKGROUND;
		}
		cout<<"["<<x<<", "<<y<<"]���state = "<<img_state.at<uchar>(focal_pixel)+0<<" �������="
			<<img_result.at<uchar>(Point(x,y))+0<<endl;
		++pixel_number;
		y = pixel_number/img_gray.cols;
		x = pixel_number%img_gray.cols;
	}
}

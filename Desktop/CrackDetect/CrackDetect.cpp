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
		cout<<endl;
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
		pixel_value->push_back(PIXEL_VALUE(img , pixels[i].x , pixels.[i].y));
	}
	return *pixel_value;
}

//T = max{ max[ I(p) ] , T } + w, 
float Func_Iterator_T(const vector<uchar> & Ip, //I(p)
									   uchar T,	//上次的亮度阈值T
									   float w)	//加速因子
{
	uchar I_max = I[0];
	for(size_t i = 1; i<I.size(); ++i)
	{
		if(I[i]>I_max)
			I_max = I[i];
	}
	if(I_max < T)
		I_max = T;
	return I_max+w;
}

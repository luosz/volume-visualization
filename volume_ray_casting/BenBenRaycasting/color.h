#ifndef _COLOR_
#define _COLOR_

#include <cmath>
using namespace std;

const float PI = 3.1415926535;

#define min3v(v1, v2, v3)   ((v1)>(v2)? ((v2)>(v3)?(v3):(v2)):((v1)>(v3)?(v3):(v2)))  
 
#define max3v(v1, v2, v3)   ((v1)<(v2)? ((v2)<(v3)?(v3):(v2)):((v1)<(v3)?(v3):(v1)))   
  
  
/**	@brief store r, g, b color component in unsigned char format 
*	
*/  
typedef struct  
  
{   

    unsigned char  red;              // [0,255]   
  
    unsigned char  green;            // [0,255]   
  
    unsigned char  blue;             // [0,255]   
  
}COLOR_RGB;  

/**	@brief store hue, saturation and luminance of HSL color space
*	
*/
typedef struct  
{   
  
    float hue;              // [0,360]   
  
    float saturation;       // [0,100]   
  
    float luminance;        // [0,100]   
  
}COLOR_HSL;   

/**	@brief convert radius of an angle to degree
*	
*/
float rad_to_deg(float r)
{
	return (r * PI / 180.0);
}

/**	@brief convert HSI triple to RGB color space 
*	
*/
void HSI_to_RGB(float H, float S, float I, float * R, float * G, float * B)
{
	float r, g, b;
	float theta;

	if(H >= 0 && H < 120)
	{
		b = (1 - S) / 3.0;
		r = 1 + S * cos(rad_to_deg(H)) / cos(rad_to_deg(60.0 - H));
		r /= 3.0;
		g = 1 - b - r;
		*R = 3 * I * r;
		*G = 3 * I * g;
		*B = 3 * I * b;
	}
	else if(H >= 120.0 && H < 240.0)
	{
		H -= 120.0;
		b = (1 - S) / 3.0;
		r = 1 + S * cos(rad_to_deg(H)) / cos(rad_to_deg(60.0 - H));
		r /= 3.0;
		g = 1 - b - r;
		*R = 3 * I * r;
		*G = 3 * I * g;
		*B = 3 * I * b;
	}
	else if(H >= 240.0 && H <= 360.0)
	{
		H -= 240.0;
		b = (1 - S) / 3.0;
		r = 1 + S * cos(rad_to_deg(H)) / cos(rad_to_deg(60.0 - H));
		r /= 3.0;
		g = 1 - b - r;
		*R = 3 * I * r;
		*G = 3 * I * g;
		*B = 3 * I * b;
	}
	else
	{
		cout<<"Wrong HSI triple"<<endl;
	}
}

/**	@brief convert HSL triple to RGB color space
*	
*/
void HSL2RGB(float H, float S, float L, float * R, float * G, float * B)
{
	float temp1, temp2, Rtemp3, Gtemp3, Btemp3;
	if(int(S * 100.0) == 0)
	 {
		 *R = *G = *B = L;
	 }
	 else
	 {
		if(int(L * 100) < 50)
			temp2 = L * (1.0 + S);
		else
			temp2 = L + S - L * S;
		temp1 = 2 * L - temp2;
		H /= 360.0;
		Rtemp3 = H + 1.0 / 3.0;
		Gtemp3 = H;
		Btemp3 = H - 1.0 / 3.0;
		if(Rtemp3 < 0.0)
			Rtemp3 += 1.0;
		if(Rtemp3 > 1.0)
			Rtemp3 -= 1.0;
		if(Gtemp3 < 0.0)
			Gtemp3 += 1.0;
		if(Gtemp3 > 1.0)
			Gtemp3 -= 1.0;
		if(Btemp3 < 0.0)
			Btemp3 += 1.0;
		if(Btemp3 > 1.0)
			Btemp3 -= 1.0;
		if(6 * Rtemp3 < 1.0)
			*R = temp1 + (temp2 - temp1) * 6.0 * Rtemp3;
		else if(2 * Rtemp3 < 1.0)
			*R = temp2;
		else if(3 * Rtemp3 < 2.0)
			*R = temp1 + (temp2 - temp1) * (2.0 / 3.0 - Rtemp3) * 6.0;
		else
			*R = temp1;
		if(6 * Gtemp3 < 1.0)
			*G = temp1 + (temp2 - temp1) * 6.0 * Gtemp3;
		else if(2 * Gtemp3 < 1.0)
			*G = temp2;
		else if(3 * Gtemp3 < 2.0)
			*G = temp1 + (temp2 - temp1) * (2.0 / 3.0 - Gtemp3) * 6.0;
		else
			*G = temp1;
		if(6 * Btemp3 < 1.0)
			*B = temp1 + (temp2 - temp1) * 6.0 * Btemp3;
		else if(2 * Btemp3 < 1.0)
			*B = temp2;
		else if(3 * Btemp3 < 2.0)
			*B = temp1 + (temp2 - temp1) * (2.0 / 3.0 - Btemp3) * 6.0;
		else
			*B = temp1;
	//	cout<<"R = "<<*R<<"G = "<<*G<<"B = "<<*B<<endl;
	 }
}

#endif
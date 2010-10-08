#include <iostream>
#include <time.h>
using namespace std;

inline unsigned int get_random(unsigned int *m_z, unsigned int *m_w)
{
	(*m_z) = 36969 * ((*m_z) & 65535) + ((*m_z) >> 16);
	(*m_w) = 18000 * ((*m_w) & 65535) + ((*m_w) >> 16);
	return ((*m_z) << 16) + (*m_w);  /* 32-bit result */
}

inline unsigned int circular_shift_right(unsigned int value, unsigned int offset, unsigned int total_bits)
{
	return (value>>offset) | (value<<(total_bits - offset));
}

void main()
{

	unsigned int t = time(0);
	cout<<t<<endl;
	cout<<hex<<t<<endl<<endl;
	cout<<(t>>8)<<endl;
	cout<<(t<<24)<<endl<<endl;
	unsigned int a = t;
	unsigned int b = (t>>8) | (t<<24);
	cout<<a<<endl<<b<<endl<<endl;

	for (unsigned int iGID=0; iGID<10; iGID++)
	{
		unsigned int random1 = a ^ iGID ^ circular_shift_right(~b, iGID % 32, 32);
		cout<<random1<<endl;
	}
}

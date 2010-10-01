#include <iostream>
#include <ctime>
#include <cstdlib>
#include <nvMath.h>
using namespace std;

// insertion sort
void insertion_sort(nv::vec4f *data, int n)
{
	nv::vec4f temp;
	for (int i=1; i<n; i++)
	{
		for (int j=i-1; j>=0; j--)
		{
			if (data[j+1].x < data[j].x)
			{
				temp = data[j];
				data[j] = data[j+1];
				data[j+1] = temp;
			}else
			{
				break;
			}
		}
	}
}

void main()
{
	const int n = 10;
	nv::vec4f data[n];

	srand( (unsigned)time( NULL ) );
	for (int i=0; i<n; i++)
	{
		data[i].x = rand() % 100;
	}

	insertion_sort(data, n);

	for (int i=0; i<n; i++)
	{
		cout<<data[i].x<<endl;
	}
}
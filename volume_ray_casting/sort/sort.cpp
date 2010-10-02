#include <iostream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <functional>
using namespace std;

/************************************************************************
Gnome Sort is based on the technique used by Dutch Garden Gnomes (Du.: tuinkabouter). Here is how a garden gnome sorts a line of flower pots. Basically, he looks at the flower pot next to him and the previous one; if they are in the right order he steps one pot forward, otherwise he swaps them and steps one pot backwards. Boundary conditions: if there is no previous pot, he steps forwards; if there is no pot next to him, he is done.
¡ªDick Grune
http://www.cs.vu.nl/~dick/gnomesort.html

Optimization
The gnome sort may be optimized by introducing a variable to store the position before traversing back toward the beginning of the list. This would allow the "gnome" to teleport back to his previous position after moving a flower pot. With this optimization, the gnome sort would become a variant of the insertion sort.
http://en.wikipedia.org/wiki/Gnome_sort
************************************************************************/

// Gnome sort, original
void gnomesort(int n, int ar[]) {
	int i = 0;

	while (i < n) {
		if (i == 0 || ar[i-1] <= ar[i]) i++;
		else {int tmp = ar[i]; ar[i] = ar[i-1]; ar[--i] = tmp;}
	}
}

// Gnome sort, improved
template<class T>
void gnome_sort(T data[], int n, bool comparator(T, T))
{
	int i = 1, previous_position = -1;
	while (i < n)
	{
		if (i > 0 && comparator(data[i], data[i-1]))
		{
			// Mark the Gnome's previous position before traverse backward
			if (previous_position == -1)
			{
				previous_position = i;
			}
			swap(data[i], data[i-1]);
			i--;
		}else
		{
			if (previous_position == -1)
			{
				i++;
			}else
			{
				// After traverse backward, go to the position next to the previous
				i = previous_position + 1;
				previous_position = -1;
			}
		}
	}
}

// Insertion sort
// http://en.wikipedia.org/wiki/Insertion_sort
template<class T>
void insertion_sort(T data[], int n, bool comparator(T, T))
{
	for (int i=1; i<n; i++)
	{
		T temp = data[i];
		int j;
		for (j=i-1; j>=0; j--)
		{
			if (comparator(temp, data[j]))
			{
				data[j+1] = data[j];
			}else
			{
				break;
			}
		}
		data[j+1] = temp;
	}
}

// Selection sort
// http://en.wikipedia.org/wiki/Selection_sort
template<class T>
void selection_sort(T data[], int n, bool comparator(T, T))
{
	for (int i=0; i<n; i++)
	{
		// Select the minimum
		int min = i;
		for (int j=i+1; j<n; j++)
		{
			if (comparator(data[j], data[min]))
			{
				min = j;
			}
		}
		if (min != i)
		{
			swap(data[min], data[i]);
		}
	}
}

// Comparator
template<class T>
bool smaller(T v1, T v2)
{
	return v1 < v2;
}

void main()
{
	const int n = 100;
	double d[n], d2[n], d3[n];

	srand( (unsigned)time( NULL ) );
	for (int i=0; i<n; i++)
	{
		d3[i] = d2[i] = d[i] = rand() % 100;
	}

	insertion_sort(d, n, smaller<double>);
	cout<<"Insertion sort"<<endl;
	for (int i=0; i<n; i++)
	{
		cout<<d[i]<<"\t";
	}
	cout<<endl;

	selection_sort(d2, n, smaller<double>);
	cout<<"Selection sort"<<endl;
	for (int i=0; i<n; i++)
	{
		cout<<d2[i]<<"\t";
	}
	cout<<endl;

	gnome_sort(d3, n, smaller<double>);
	cout<<"Gnome sort"<<endl;
	for (int i=0; i<n; i++)
	{
		cout<<d3[i]<<"\t";
	}
	cout<<endl;
}
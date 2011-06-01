
//--------------------------------------------------------------------------
// The code given here is taken from "Numerical Recipes in C++ (3rd 2007)" 
// by William Press, Brian Flannery, Saul Teukolsky, and William Vetterling.
//--------------------------------------------------------------------------

#ifndef NUMERIC_RAND_H
#define NUMERIC_RAND_H

typedef int Int; // 32 bit integer
typedef unsigned int Uint;

#ifdef _MSC_VER
typedef __int64 Llong; // 64 bit integer
typedef unsigned __int64 Ullong;
#else
typedef long long int Llong; // 64 bit integer
typedef unsigned long long int Ullong;
#endif

typedef char Char; // 8 bit integer
typedef unsigned char Uchar;

typedef double Doub; // default floating type
typedef long double Ldoub;

typedef bool Bool;

struct Ran 
{
	Ullong u,v,w;
	Ran(Ullong j) : v(4101842887655102017LL), w(1) 
	{
		u = j ^ v; int64();
		v = u; int64();
		w = v; int64();
	}

	// returns a random 64-bit unsigned integer
	inline Ullong int64() 
	{
		u = u * 2862933555777941757LL + 7046029254386353087LL;
		v ^= v >> 17; v ^= v << 31; v ^= v >> 8;
		w = 4294957665U*(w & 0xffffffff) + (w >> 32);
		Ullong x = u ^ (u << 21); x ^= x >> 35; x ^= x << 4;
		return (x + v) ^ w;
	}

	// returns a double-precision floating value in the range 0:0 to 1:0.
	inline Doub doub()  { return 5.42101086242752217E-20 * int64(); }

	// returns an unsigned 32-bit integer
	inline Uint int32() { return (Uint)int64(); }

	inline Doub doub(Doub low, Doub high) //  [8/1/2010 jh.wu@siat.ac.cn]
	{
		return low + doub()* (high - low);
	}

	// return a random integer between low and high(inclusive)
	inline Uint int32(Uint low, Uint high)
	{
		return low + int32()%(high-1);
	}
};


struct Ranq1 
{
	Ullong v;
	Ranq1(Ullong j) : v(4101842887655102017LL) 
	{
		v ^= j;
		v = int64();
	}
	inline Ullong int64() 
	{
		v ^= v >> 21; v ^= v << 35; v ^= v >> 4;
		return v * 2685821657736338717LL;
	}
	inline Doub doub() { return 5.42101086242752217E-20 * int64(); }
	inline Uint int32() { return (Uint)int64(); }
};

struct Ranq2 
{
	Ullong v,w;
	Ranq2(Ullong j) : v(4101842887655102017LL), w(1) 
	{
		v ^= j;
		w = int64();
		v = int64();
	}
	inline Ullong int64() 
	{
		v ^= v >> 17; v ^= v << 31; v ^= v >> 8;
		w = 4294957665U*(w & 0xffffffff) + (w >> 32);
		return v ^ w;
	}

	inline Doub doub() { return 5.42101086242752217E-20 * int64(); }
	inline Uint int32() { return (Uint)int64(); }
};

struct Ranhash 
{
	inline Ullong int64(Ullong u) 
	{
		Ullong v = u * 3935559000370003845LL + 2691343689449507681LL;
		v ^= v >> 21; v ^= v << 37; v ^= v >> 4;
		v *= 4768777513237032717LL;
		v ^= v << 20; v ^= v >> 41; v ^= v << 5;
		return  v;
	}

	inline Uint int32(Ullong u)
	{ return (Uint)(int64(u) & 0xffffffff) ; }

	inline Doub doub(Ullong u)
	{ return 5.42101086242752217E-20 * int64(u); }
};

struct Ranbyte 
{
	Int s[256],i,j,ss;
	Uint v;

	Ranbyte(Int u) 
	{
		v = 2244614371U ^ u;
		for (i=0; i<256; i++) {s[i] = i;}
		for (j=0, i=0; i<256; i++) {
			ss = s[i];
			j = (j + ss + (v >> 24)) & 0xff;
			s[i] = s[j]; s[j] = ss;
			v = (v << 24) | (v >> 8);
		}
		i = j = 0;
		for (Int k=0; k<256; k++) int8();
	}

	inline unsigned char int8() 
	{
		i = (i+1) & 0xff;
		ss = s[i];
		j = (j+ss) & 0xff;
		s[i] = s[j]; s[j] = ss;
		return (unsigned char)(s[(s[i]+s[j]) & 0xff]);
	}

	Uint int32() 
	{
		v = 0;
		for (int k=0; k<4; k++) 
		{
			i = (i+1) & 0xff;
			ss = s[i];
			j = (j+ss) & 0xff;
			s[i] = s[j]; s[j] = ss;
			v = (v << 8) | s[(s[i]+s[j]) & 0xff];
		}
		return v;
	}

	Doub doub() 
	{
		return 2.32830643653869629E-10 * ( int32() +
			2.32830643653869629E-10 * int32() );
	}
};

struct Ranfib 
{
	Doub dtab[55], dd;
	Int inext, inextp;
	Ranfib(Ullong j) : inext(0), inextp(31) {
		Ranq1 init(j);
		for (int k=0; k<55; k++) dtab[k] = init.doub();
	}
	Doub doub() {
		if (++inext == 55) inext = 0;
		if (++inextp == 55) inextp = 0;
		dd = dtab[inext] - dtab[inextp];
		if (dd < 0) dd += 1.0;
		return (dtab[inext] = dd);
	}
	inline unsigned long int32()
	{ return (unsigned long)(doub() * 4294967295.0);}
};

struct Ranlim32 
{
	Uint u,v,w1,w2;
	Ranlim32(Uint j) : v(2244614371U), w1(521288629U), w2(362436069U) 
	{
		u = j ^ v; int32();
		v = u; int32();
	}

	inline Uint int32() 
	{
		u = u * 2891336453U + 1640531513U;
		v ^= v >> 13; v ^= v << 17; v ^= v >> 5;
		w1 = 33378 * (w1 & 0xffff) + (w1 >> 16);
		w2 = 57225 * (w2 & 0xffff) + (w2 >> 16);
		Uint x = u ^ (u << 9); x ^= x >> 17; x ^= x << 6;
		Uint y = w1 ^ (w1 << 17); y ^= y >> 15; y ^= y << 5;
		return (x + v) ^ (y + w2);
	}
	inline Doub doub() { return 2.32830643653869629E-10 * int32(); }
	inline Doub truedoub() {
		return 2.32830643653869629E-10 * ( int32() +
			2.32830643653869629E-10 * int32() );
	}
};



#endif // NUMERIC_RAND_H

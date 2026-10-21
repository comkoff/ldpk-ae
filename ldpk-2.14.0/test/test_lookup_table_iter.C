// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#include <ldpk/ldpk.h>
#include <sstream>

using namespace std;
using namespace ldpk;

// This program demonstrates how iterating is done in the lookup table.

int main()
	{
	lookup_table<vec2d> lut;
	stringstream sout;
	string s[7 * 9 * 7];

// Initializing
	lut.init(9,7);
	lut.set_lens_center_fov(vec2d(.4,.3));
// Iterating
	lut.reset();
	while(lut.next())
		{
		lut.out_state(sout);
		}

// Only for pretty printing
	for(int i = 0;i < 7 * 9 * 7;++i)
		{
		sout >> s[i];
		}
	for(int j = 0;j < 7;++j)
		{
		for(int i = 0;i < 7;++i)
			{
			for(int k = 0;k < 9;++k)
				{
				cout << s[i + 7 * 9 * j + 7 * k] << "     ";
				}
			cout << endl;
			}
		cout << endl;
		}
	}

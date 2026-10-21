// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#include <ldpk/ldpk_generic_radial_distortion_1d.h>

using namespace std;
using namespace ldpk;

// A test program for generic radial distortion.

int main()
	{
	ldpk::generic_radial_distortion_1d<4> model;

// Specify distortion coefficients
	model.set_coeff(0,-.0032);
	model.set_coeff(1,.0160);
	model.set_coeff(2,-.0256);
	model.set_coeff(3,.0128);

// Sample points
	int n = 50;
	double diff_max = 0.0;
	for(int i = 1;i <= n;++i)
		{
		double r_unit = double(i) / n;
		double r_undistort = model(r_unit);
		double r_redistort = model.map_inverse(r_undistort,r_undistort);
		double diff = ::fabs(r_redistort - r_unit);
		if(diff > diff_max)
			{ diff_max = diff; }
		cout << r_unit / r_unit << " " << r_undistort / r_unit << " " << r_redistort / r_unit << endl;
		}
	cerr << "diff_max: " << diff_max << endl;
	}

// g++ test_generic_radial_distortion_1d.C -g -I ../include/ -o test_generic_radial_distortion_1d

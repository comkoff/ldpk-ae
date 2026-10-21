// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#include <ldpk/ldpk_vec3d.h>

int main()
	{
	ldpk::vec3d a(2,3,5);
	ldpk::vec3d b(-7,11,-13);

// Axis of rotation
	ldpk::vec3d axis = wdg(a,b);
// Angle that rotates a into b.
	double phi = ::atan2(norm2(axis),dot(a,b));

// This test is crucial for the stabilizer models.
	ldpk::mat3d r = make_rot3_from_angle_and_axis(phi,axis);
	std::cout << ldpk::unit(b) << std::endl;
	std::cout << unit(r * a) << std::endl;
// Check if it's really a rotation matrix.
	std::cout << det(r) << std::endl;
	}

// Compile like
// g++ -g test_ldpk_vec3.C -I ../../include/ -o test_ldpk_vec3

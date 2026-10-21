// LDPK-to-AfterEffects bridge.
// Console tool: parse an LD_3DE4 node block from a .nk file, build the
// LDPK model and print a grid of mapped unit coordinates. Used to verify
// the parser and the model wiring against Nuke renders.
//
// Build (x64 Native Tools Command Prompt):
//   cl /nologo /EHsc /MD /D_WINDOWS /I..\include nk_ld_dump.cpp /Fe:nk_ld_dump.exe
//
// Usage:
//   nk_ld_dump.exe <file.nk> [frame]
//
// Software is provided "as is" - no warranties implied.

#include "ldpk_nk_loader.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(int argc,char** argv)
	{
	if(argc < 2)
		{
		std::cerr << "usage: nk_ld_dump <file.nk> [frame]" << std::endl;
		return 1;
		}
	int frame = (argc >= 3) ? atoi(argv[2]) : 1;

	ae_ldpk::distortion_from_nk d;
	std::string err = d.load(argv[1],frame);
	if(!err.empty())
		{
		std::cerr << "error: " << err << std::endl;
		return 1;
		}
	const ae_ldpk::nk_ld_node& n = d.node();
	std::cout << "class    : " << n.nuke_class << std::endl;
	std::cout << "name     : " << n.name << std::endl;
	std::cout << "direction: " << n.direction << std::endl;
	std::cout << "params   :" << std::endl;
	for(std::map<std::string,ae_ldpk::nk_value>::const_iterator i = n.params.begin();i != n.params.end();++i)
		{
		std::cout << "  " << i->first << " = ";
		if(i->second.animated)
			{
			std::cout << "{curve";
			for(size_t k = 0;k < i->second.keys.size();++k)
				{ std::cout << " x" << i->second.keys[k].first << " " << i->second.keys[k].second; }
			std::cout << "} @frame " << frame << " -> " << i->second.eval(frame);
			}
		else
			{ std::cout << i->second.static_value; }
		std::cout << std::endl;
		}
// Print a 5x5 grid of mapped coordinates (unit coordinates, like the Nuke node).
	std::cout << "map grid (unit coords, " << (d.is_undistort() ? "undistort" : "distort") << "):" << std::endl;
	for(int iy = 0;iy <= 4;++iy)
		{
		double y = double(iy) / 4.0;
		for(int ix = 0;ix <= 4;++ix)
			{
			double x = double(ix) / 4.0;
			double qx,qy;
			if(d.apply(x,y,qx,qy))
				{ std::printf("  (%.4f,%.4f) -> (%.6f,%.6f)\n",x,y,qx,qy); }
			else
				{ std::printf("  (%.4f,%.4f) -> (undefined)\n",x,y); }
			}
		}
	return 0;
	}

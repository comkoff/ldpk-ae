// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2021,  Science-D-Visions. Current version: 2.5

#include <ldpk/nuke_ld_3de4_base.h>
#include <ldpk/tde4_ldp_radial_degree_6_decentered_degree_2_opencv.h>

#define CLASS_LDP_SDV         tde4_ldp_radial_degree_6_decentered_degree_2_opencv
#define CLASS_IOP_SDV     iop_tde4_ldp_radial_degree_6_decentered_degree_2_opencv
#define CREATE_SDV create_iop_tde4_ldp_radial_degree_6_decentered_degree_2_opencv

//! @file

// This is the name that NUKE will use to store this operator in the
// scripts. So that NUKE can locate the plugin, this must also be the
// name of the compiled plugin (with .so/.dll/.dylib added to the end):
static const char* const CLASS = "LD_OpenCV_Radial_Degree_6_Decentered_Degree_2";

// This text will be displayed in a popup help box on the node's panel:
static const char* const HELP = "OpenCV radial lens distortion model, which compensates for decentered lenses.\n";

//! Nuke plugin based on tde4_ldp_radial_degree_6_decentered_degree_2_opencv
class iop_tde4_ldp_radial_degree_6_decentered_degree_2_opencv:public nuke_ld_3de4_base
	{
public:
	CLASS_IOP_SDV(Node* node):nuke_ld_3de4_base(node,new CLASS_LDP_SDV<ldpk::vec2d,ldpk::mat2d>())
		{
		if(_verbose_constructor)
			{
			std::cout << "iop_tde4_ldp_radial_degree_6_decentered_degree_2_opencv::ctor: node: " << node << std::endl;
			std::cout << "iop_tde4_ldp_radial_degree_6_decentered_degree_2_opencv::ctor: this: " << this << std::endl;
			}
		}
	~CLASS_IOP_SDV()
		{
		if(_verbose_destructor)
			{
			std::cout << "iop_tde4_ldp_radial_degree_6_decentered_degree_2_opencv::dtor: this: " << this << std::endl;
			}
		}
	const char* Class() const
		{ return CLASS; }
	const char* node_help() const
		{ return HELP; }
	};

// So wird das woanders auch gemacht. Bauen wir einfach nach.
static DD::Image::Iop* CREATE_SDV(Node* node)
	{
	return (new DD::Image::NukeWrapper (new CLASS_IOP_SDV(node)))->noMix()->noMask();
	}
const DD::Image::Iop::Description nuke_ld_3de4_base::description(CLASS,0,CREATE_SDV);

// g++ -g -O2 -shared -fPIC nuke_ld_3de4_radial_standard_degree_4.C nuke_ld_3de4_base.C -I $server/software/linux64/nuke/Nuke10.0v4/include -I ../../include -L $server/software/linux64/nuke/Nuke10.0v4/lib -o ../../compiled/nuke/linux/Nuke10.0/LD_3DE4_Radial_Standard_Degree_4.so

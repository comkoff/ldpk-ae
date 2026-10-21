// Round-trip test for the sequence-data snapshot and the
// .nk script scanner, plus an apply/apply_checked consistency
// check in fov coordinates.
//
// Build (see readme.md):
//   cl /nologo /EHsc /MD /D_WINDOWS /I..\include test_snapshot_roundtrip.cpp /Fe:test_snapshot_roundtrip.exe
// Run:
//   test_snapshot_roundtrip.exe <file.nk>   (standalone block or full Nuke script)
//
// Software is provided "as is" - no warranties implied.

#include "ldpk_nk_loader.h"

#include <cstdio>
#include <fstream>
#include <vector>

static bool expect(bool cond, const char* what)
	{
	std::printf("%s : %s\n",cond ? "OK  " : "FAIL",what);
	return cond;
	}

int main(int argc,char** argv)
	{
	if(argc < 2)
		{ std::printf("usage: test_snapshot_roundtrip <file.nk>\n");return 2; }
	bool all = true;

// 1. Parse (standalone exporter block or a full Nuke script).
	std::ifstream in(argv[1]);
	ae_ldpk::nk_ld_node node;
	std::string err;
	all &= expect(ae_ldpk::parse_nk_script(in,node,err),err.empty() ? "parse_nk_script" : err.c_str());
	if(!all)
		{ return 1; }
	std::printf("      class='%s' name='%s' direction='%s' params=%d\n",
		node.nuke_class.c_str(),node.name.c_str(),node.direction.c_str(),(int)node.params.size());

// 2. Serialize -> deserialize -> serialize: byte-identical blob.
	std::vector<unsigned char> blob1,blob2;
	all &= expect(node.serialize(blob1,err),"serialize");
	ae_ldpk::nk_ld_node node2;
	all &= expect(node2.deserialize(&blob1[0],blob1.size(),err),err.empty() ? "deserialize" : err.c_str());
	all &= expect(node2.serialize(blob2,err),"re-serialize");
	all &= expect(blob1 == blob2,"round-trip is byte-identical");
	all &= expect((node2.nuke_class == node.nuke_class) && (node2.name == node.name) && (node2.direction == node.direction),"strings survive");
	all &= expect(node2.params.size() == node.params.size(),"param count survives");
	size_t keys1 = 0,keys2 = 0;
	for(std::map<std::string,ae_ldpk::nk_value>::const_iterator i = node.params.begin();i != node.params.end();++i)
		{ keys1 += i->second.keys.size(); }
	for(std::map<std::string,ae_ldpk::nk_value>::const_iterator i = node2.params.begin();i != node2.params.end();++i)
		{ keys2 += i->second.keys.size(); }
	all &= expect(keys1 == keys2,"keyframe count survives");
	std::printf("      blob = %d bytes, %d keyframes\n",(int)blob1.size(),(int)keys1);

// 3. A truncated blob must be rejected (bounds-checked reader).
	if(blob1.size() > 8)
		{
		ae_ldpk::nk_ld_node node3;
		all &= expect(!node3.deserialize(&blob1[0],blob1.size() - 8,err),"truncated blob rejected");
		}

// 4. adopt + apply/apply_checked consistency: both paths work
// in the same fov coordinates, so for well-behaved models they must
// agree on a grid of points; apply_checked may only ADD rejections
// (diverged Newton), never shift the geometry.
	ae_ldpk::distortion_from_nk dist;
	all &= expect(dist.adopt(node,1) == "","adopt");
	int agree = 0,mismatch = 0,failed = 0;
	for(int iy = 0;iy <= 20;++iy)
		{
		for(int ix = 0;ix <= 20;++ix)
			{
			double x0 = ix / 20.0,y0 = iy / 20.0;
			double q1x,q1y,q2x,q2y;
			bool ok1 = dist.apply(x0,y0,q1x,q1y);
			bool ok2 = dist.apply_checked(x0,y0,q2x,q2y);
			if(ok1 && ok2)
				{
				const double dx = q1x - q2x,dy = q1y - q2y;
				if((dx * dx + dy * dy) < 1e-12)
					{ ++agree; }
				else
					{ ++mismatch; }
				}
			else if(ok2)
				{ ++mismatch; }
			else
				{ ++failed; }
			}
		}
	std::printf("      apply vs apply_checked: agree=%d mismatch=%d rejected=%d\n",agree,mismatch,failed);
	all &= expect(mismatch == 0,"no geometry shift between apply and apply_checked");
	all &= expect(failed == 0,"no unexpected rejections on a test lens");

	std::printf(all ? "ALL OK\n" : "FAILURES PRESENT\n");
	return all ? 0 : 1;
	}

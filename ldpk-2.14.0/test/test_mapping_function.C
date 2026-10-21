// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#include <ldpk/ldpk_mapping_function.h>


void test_mapping_function_equisolid()
	{
	ldpk::mapping_function_equisolid<ldpk::vec2d,ldpk::mat2d> mf;
	double eps = 1e-4;
	double r = 0.1,r_un,r_re,dr_un;
	double f = 0.7;
	mf.set_fl_dn(f);
	mf.set_scale(1.05);
	std::cout << "***** mapping_function_equisolid *****" << std::endl;
	std::cout << mf.remap_any2plain(r,r_un) << std::endl;
	std::cout << mf.remap_plain2any(r_un,r_re) << std::endl;
	std::cout << mf.derive_any2plain(r,dr_un) << std::endl;
	std::cout << "   r: " << r << std::endl;
	std::cout << "r_un: " << r_un << std::endl;
/***** inverse test *******************************************/
	std::cout << "****** inverse test ********************************************" << std::endl;
	std::cout << "r_re: " << r_re << std::endl;
/***** clip test **********************************************/
	std::cout << "****** clip test ***********************************************" << std::endl;
	mf.set_r_plain_dn_clip(10.0);
	std::cout << "theshold_valid: " << mf.threshold_valid() << std::endl;
	std::cout << "theshold_clipped: " << mf.threshold_clipped() << std::endl;
	for(int i = -5;i <= 5;++i)
		{
		double r = 1.0 + i / 50.0;
		double r_un = mf.undistort(r);
		double dr_un = mf.jacobi(ldpk::vec2d(r,0.0))[0][0];
		std::cout << "r = " << r << ": valid, clipped: " << mf.valid() << "," << mf.clipped() << ", r_un: " << r_un << ", dr_un: " << dr_un << std::endl;
		}
/***** derivative vs diffquot test ****************************/
	std::cout << "****** derivative vs diffquot test *****************************" << std::endl;
	double r_un_a,r_un_b;
	mf.remap_any2plain(r + eps,r_un_b);
	mf.remap_any2plain(r - eps,r_un_a);
	std::cout << (r_un_b - r_un_a) / (2.0 * eps) << std::endl;
	std::cout << dr_un << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(-r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,-r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r)) << std::endl;
	}
void test_mapping_function_equidistant()
	{
	ldpk::mapping_function_equidistant<ldpk::vec2d,ldpk::mat2d> mf;
	double eps = 1e-4;
	double r = 0.1,r_un,r_re,dr_un;
	double f = 0.9;
	mf.set_fl_dn(f);
	mf.set_scale(1.05);
	std::cout << "***** mapping_function_equidistant *****" << std::endl;
	std::cout << mf.remap_any2plain(r,r_un) << std::endl;
	std::cout << mf.remap_plain2any(r_un,r_re) << std::endl;
	std::cout << mf.derive_any2plain(r,dr_un) << std::endl;
	std::cout << "   r: " << r << std::endl;
	std::cout << "r_un: " << r_un << std::endl;
/***** inverse test *******************************************/
	std::cout << "****** inverse test ********************************************" << std::endl;
	std::cout << "r_re: " << r_re << std::endl;
/***** clip test **********************************************/
	std::cout << "****** clip test ***********************************************" << std::endl;
	mf.set_r_plain_dn_clip(10.0);
	std::cout << "theshold_valid: " << mf.threshold_valid() << std::endl;
	std::cout << "theshold_clipped: " << mf.threshold_clipped() << std::endl;
	for(int i = -5;i <= 5;++i)
		{
		double r = 1.35 + i / 50.0;
		double r_un = mf.undistort(r);
		double dr_un = mf.jacobi(ldpk::vec2d(r,0.0))[0][0];
		std::cout << "r = " << r << ": valid, clipped: " << mf.valid() << "," << mf.clipped() << ", r_un: " << r_un << ", dr_un: " << dr_un << std::endl;
		}
/***** derivative vs diffquot test ****************************/
	std::cout << "****** derivative vs diffquot test *****************************" << std::endl;
	double r_un_a,r_un_b;
	mf.remap_any2plain(r + eps,r_un_b);
	mf.remap_any2plain(r - eps,r_un_a);
	std::cout << (r_un_b - r_un_a) / (2.0 * eps) << std::endl;
	std::cout << dr_un << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(-r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,-r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r)) << std::endl;
	}
void test_mapping_function_orthographic()
	{
	ldpk::mapping_function_orthographic<ldpk::vec2d,ldpk::mat2d> mf;
	double eps = 1e-4;
	double r = 0.1,r_un,r_re,dr_un;
	double f = 0.9;
	mf.set_fl_dn(f);
	mf.set_scale(1.05);
	std::cout << "***** mapping_function_orthographic *****" << std::endl;
	std::cout << mf.remap_any2plain(r,r_un) << std::endl;
	std::cout << mf.remap_plain2any(r_un,r_re) << std::endl;
	std::cout << mf.derive_any2plain(r,dr_un) << std::endl;
	std::cout << "   r: " << r << std::endl;
	std::cout << "r_un: " << r_un << std::endl;
/***** inverse test *******************************************/
	std::cout << "****** inverse test ********************************************" << std::endl;
	std::cout << "r_re: " << r_re << std::endl;
/***** clip test **********************************************/
	std::cout << "****** clip test ***********************************************" << std::endl;
	mf.set_r_plain_dn_clip(5.0);
	std::cout << "theshold_valid: " << mf.threshold_valid() << std::endl;
	std::cout << "theshold_clipped: " << mf.threshold_clipped() << std::endl;
	for(int i = -5;i <= 5;++i)
		{
		double r = 0.9 + i / 100.0;
		double r_un = mf.undistort(r);
		double dr_un = mf.jacobi(ldpk::vec2d(r,0.0))[0][0];
		std::cout << "r = " << r << ": valid, clipped: " << mf.valid() << "," << mf.clipped() << ", r_un: " << r_un << ", dr_un: " << dr_un << std::endl;
		}
/***** derivative vs diffquot test ****************************/
	std::cout << "****** derivative vs diffquot test *****************************" << std::endl;
	double r_un_a,r_un_b;
	mf.remap_any2plain(r + eps,r_un_b);
	mf.remap_any2plain(r - eps,r_un_a);
	std::cout << (r_un_b - r_un_a) / (2.0 * eps) << std::endl;
	std::cout << dr_un << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(-r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,-r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r)) << std::endl;
	}
void test_mapping_function_stereographic()
	{
	ldpk::mapping_function_stereographic<ldpk::vec2d,ldpk::mat2d> mf;
	double eps = 1e-4;
	double r = 0.1,r_un,r_re,dr_un;
	double f = 0.9;
	mf.set_fl_dn(f);
	mf.set_scale(1.05);
	std::cout << "***** mapping_function_stereographic *****" << std::endl;
	std::cout << mf.remap_any2plain(r,r_un) << std::endl;
	std::cout << mf.remap_plain2any(r_un,r_re) << std::endl;
	std::cout << mf.derive_any2plain(r,dr_un) << std::endl;
	std::cout << "   r: " << r << std::endl;
	std::cout << "r_un: " << r_un << std::endl;
/***** inverse test *******************************************/
	std::cout << "****** inverse test ********************************************" << std::endl;
	std::cout << "r_re: " << r_re << std::endl;
/***** clip test **********************************************/
	std::cout << "****** clip test ***********************************************" << std::endl;
	mf.set_r_plain_dn_clip(5.0);
	std::cout << "theshold_valid: " << mf.threshold_valid() << std::endl;
	std::cout << "theshold_clipped: " << mf.threshold_clipped() << std::endl;
	for(int i = -5;i <= 5;++i)
		{
		double r = 1.6 + i / 25.0;
		double r_un = mf.undistort(r);
		double dr_un = mf.jacobi(ldpk::vec2d(r,0.0))[0][0];
		std::cout << "r = " << r << ": valid, clipped: " << mf.valid() << "," << mf.clipped() << ", r_un: " << r_un << ", dr_un: " << dr_un << std::endl;
		}
/***** derivative vs diffquot test ****************************/
	std::cout << "****** derivative vs diffquot test *****************************" << std::endl;
	double r_un_a,r_un_b;
	mf.remap_any2plain(r + eps,r_un_b);
	mf.remap_any2plain(r - eps,r_un_a);
	std::cout << (r_un_b - r_un_a) / (2.0 * eps) << std::endl;
	std::cout << dr_un << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(-r,0.0))[0][0] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,-r))[1][1] << std::endl;
	std::cout << mf.jacobi(ldpk::vec2d(0.0,r)) << std::endl;
	}

int main()
	{
	test_mapping_function_equisolid();
	test_mapping_function_equidistant();
	test_mapping_function_orthographic();
	test_mapping_function_stereographic();
	}

// g++ -g test_mapping_function.C -I ../include/ -o test_mapping_function

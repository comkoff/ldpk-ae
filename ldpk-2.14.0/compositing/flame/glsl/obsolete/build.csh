#!/bin/csh

python3 expand_files.py prg_1_tex_tde4_anamorphic_standard_degree_4.fsh expand.xpnd			> ../2019.1/LD_3DE4_Anamorphic_Standard_Degree_4.glsl
python3 expand_files.py prg_1_tex_tde4_anamorphic_rescaled_degree_4.fsh expand.xpnd			> ../2019.1/LD_3DE4_Anamorphic_Rescaled_Degree_4.glsl
python3 expand_files.py prg_1_tex_tde4_anamorphic_standard_degree_6.fsh expand.xpnd			> ../2019.1/LD_3DE4_Anamorphic_Standard_Degree_6.glsl
python3 expand_files.py prg_1_tex_tde4_anamorphic_rescaled_degree_6.fsh expand.xpnd			> ../2019.1/LD_3DE4_Anamorphic_Rescaled_Degree_6.glsl
python3 expand_files.py prg_1_tex_tde4_anamorphic_stabilizer_degree_6.fsh expand.xpnd			> ../2019.1/LD_3DE4_Anamorphic_Stabilizer_Degree_6.glsl
python3 expand_files.py prg_1_tex_tde4_radial_standard_degree_4.fsh expand.xpnd				> ../2019.1/LD_3DE4_Radial_Standard_Degree_4.glsl
python3 expand_files.py prg_1_tex_tde4_radial_stabilizer_degree_6.fsh expand.xpnd			> ../2019.1/LD_3DE4_Radial_Stabilizer_Degree_6.glsl
python3 expand_files.py prg_1_tex_tde4_brown_conrady_degree_6_2.fsh expand.xpnd				> ../2019.1/LD_3DE4_Brown_Conrady_Degree_6_2.glsl
python3 expand_files.py prg_1_tex_tde4_classic_3de_mixed.fsh expand.xpnd				> ../2019.1/LD_3DE_Classic_LD_Model.glsl
python3 expand_files.py prg_1_tex_tde4_radial_fisheye_degree_8.fsh expand.xpnd				> ../2019.1/LD_3DE4_Radial_Fisheye_Degree_8.glsl

#shader_builder -p -m ld_3de4_anamorphic_standard_deg_4.glsl

chmod 775 ../2019.1/*.glsl

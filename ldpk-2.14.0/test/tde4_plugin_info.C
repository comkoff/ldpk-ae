// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#include <ldpk/ldpk_plugin_loader.h>
#include <iostream>
#include <dlfcn.h>
#include <stdlib.h>

// The executable tde4_plugin_info is not meant to be self-compiled,
// since it contains sdv-internal sources, like this one:
#include <tab/tab2.h>

using namespace std;
using namespace ldpk;

int main(int argc,char** argv)
	{
	if(argc != 2)
		{
		cerr << "usage: test_plugin_loader </preferably/absolute/path/to/plugin.so>" << endl;
		exit(-1);
		}
	plugin_loader l;
	l.open_plugin(argv[1]);
	if(l.get_model())
		{
// Write plugin info to stdout, close and exit.
		int num_parameters;
		l.get_model()->getNumParameters(num_parameters);

		char model_name[100];
		l.get_model()->getModelName(model_name);

		tb::table tab;

		tab << tb::row << tb::caption << tb::begin << "plugin: '" << model_name << "'" << tb::end << tb::sep;
		tab << tb::row << tb::caption << tb::begin << num_parameters << " model-dependent parameter" << (num_parameters != 1 ? "s" : "") << tb::end;
		tab << tb::row << "parameter" << "type" << "default value" << "range" << tb::sep;

		for(int i = 0;i < num_parameters;++i)
			{
			char parameter_name[100];
			tde4_ldp_ptype parameter_type;

			l.get_model()->getParameterName(i,parameter_name);
			l.get_model()->getParameterType(parameter_name,parameter_type);
			if(parameter_type == TDE4_LDP_ADJUSTABLE_DOUBLE)
				{
				double d,a,b;
				l.get_model()->getParameterRange(parameter_name,a,b);
				l.get_model()->getParameterDefaultValue(parameter_name,d);
				tab << tb::row << parameter_name << "adjustable double" << d << tb::begin << a << " to " << b << tb::end;
				}
			else if(parameter_type == TDE4_LDP_DOUBLE)
				{
				double d;
				l.get_model()->getParameterDefaultValue(parameter_name,d);
				tab << tb::row << parameter_name << "double" << d << "";
				}
			else if(parameter_type == TDE4_LDP_STRING)
				{
				char d[1024];
				l.get_model()->getParameterDefaultValue(parameter_name,d);
				tab << tb::row << parameter_name << "string" << d << "";
				}
			else if(parameter_type == TDE4_LDP_INT)
				{
				int d;
				l.get_model()->getParameterDefaultValue(parameter_name,d);
				tab << tb::row << parameter_name << "int" << d << "";
				}
			else if(parameter_type == TDE4_LDP_TOGGLE)
				{
				bool d;
				l.get_model()->getParameterDefaultValue(parameter_name,d);
				tab << tb::row << parameter_name << "bool" << tb::begin << std::boolalpha << d << std::noboolalpha << tb::end << "";
				}
			else if(parameter_type == TDE4_LDP_FILE)
				{
				char d[4096];
				l.get_model()->getParameterDefaultValue(parameter_name,d);
				tab << tb::row << parameter_name << "filename" << d << "";
				}
			}

		std::cout << tab << std::endl;

		l.close_plugin();
		}
	}


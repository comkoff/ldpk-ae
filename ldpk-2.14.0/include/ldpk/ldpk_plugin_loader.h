// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/tde4_ld_plugin.h>
#include <string>

#if defined _SCI_OS_WIN32
	#include <incwrap/dlfcn.h>
// In the LDPK this must compile without our internal file structure
#else
	#if defined _WIN32
		#include <dlfcn.h>
	#else
		#include <dlfcn.h>
	#endif
#endif

//! @file ldpk_plugin_loader.h
//! @brief A simple plugin loader.

namespace ldpk
	{
//! @brief A simple plugin loader for testing and debugging purposes.
	class plugin_loader
		{
	private:
		tde4ldp_create_fct_t* _create_plugin;
		tde4ldp_destroy_fct_t* _destroy_plugin;
		tde4_ld_plugin* _model;
// Mode for dlopen, default is RTLD_LAZY
		int _mode;
		bool _verbose;
	public:
		plugin_loader(bool verbose = true):_create_plugin(0),_destroy_plugin(0),_model(0),_verbose(verbose),_mode(RTLD_LAZY)
			{ }
		~plugin_loader()
			{ close_plugin(); }
//! @brief For a list of possible value, see man dlopen.
//! Call this only if you are familiar with dlopen(). Default is RTLD_LAZY.
		void set_mode(int mode)
			{ _mode = mode; }
//! @brief Load and link plugin, will throw exceptions on fail
		void open_plugin(const std::string& path);
//! @brief Close plugin, will be called by destructor.
		void close_plugin();
//! @brief Pointer to distortion model base class.
		tde4_ld_plugin* get_model()
			{ return _model; }
//! @brief The create-function for this plugin
		tde4ldp_create_fct_t* get_create_function()
			{ return _create_plugin; }
//! @brief The destroy-function for this plugin
		tde4ldp_destroy_fct_t* get_destroy_function()
			{ return _destroy_plugin; }
		};
	};


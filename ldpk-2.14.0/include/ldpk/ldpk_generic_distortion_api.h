// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/ldpk_error.h>

namespace ldpk
	{
//! @brief For organizing the documentation. This class is not really
//! required and there are not many common functions, but this may change
//! in the future.
	class generic_distortion_api
		{
	public:
		virtual ~generic_distortion_api()
			{ }
		virtual int get_num_parameters() const = 0;
//! @brief There must be methods to address coefficients by one single index i in [0,N[
		virtual double get_coeff(int i) const = 0;
		virtual void set_coeff(int i,double) = 0;
		};
	}

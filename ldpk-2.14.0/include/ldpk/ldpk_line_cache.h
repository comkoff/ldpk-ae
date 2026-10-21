// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file
//! In order to solve the glibc-problem, we replace calls to
//! std::vector<T>::at by operator[]. By doing so we make the
//! Nuke plugins running on Centos 7 which is widely spread.

#ifdef _WINDOWS
#else
#include <pthread.h>
#endif

#include <string.h>
#include <stdexcept>
#include <vector>
#include <map>
#include <iostream>

namespace ldpk
	{
	template <class T>
	class extendible_vector
		{
	public:
		typedef extendible_vector this_type;
		typedef std::vector<T> vec_type;
		typedef typename vec_type::const_iterator const_iterator;
		typedef typename vec_type::const_reverse_iterator const_reverse_iterator;
	private:
		int _a,_b;
		vec_type _val;
	public:
		extendible_vector():_a((1LL << 31) - 1),_b(-(1LL << 31))
			{ }
		extendible_vector(const this_type& c):_a(c._a),_b(c._b)
			{ _val = c._val; }
		void clear()
			{
			_a = (1LL << 31) - 1;
			_b = -(1LL << 31);
			_val.clear();
			}
//! @brief After this call (*this)[i] is valid for a <= i < b. Expensive.
//! This method will only do something if [a,b[ does not lie within [_a,_b[.
//! If [a,b[ is larger than _val.size(), resize() is invoked for _val.
		this_type& resize(int a,int b)
			{
// Invalid, ignore.
			if(a >= b)
				{ return *this; }
// New interval within old interval, nothing to do.
			if((a >= _a) && (b <= _b))
				{ return *this; }
// Is this the first call?
			if(_a > _b)
				{
				_a = a;
				_b = b;
				_val.resize(_b - _a,T());
				return *this;
				}
			else
				{
// This part might be expensive. We insert at front and back, as needed.
				if(a < _a)
					{ _val.insert(_val.begin(),_a - a,T()); }
				if(b > _b)
					{ _val.insert(_val.end(),b - _b,T()); }
				_a = std::min(a,_a);
				_b = std::max(b,_b);
				return *this;
				}
// unreachable
			return *this;
			}
//! @brief After this call (*this)[i] is valid.
		void extend(int i)
			{
			int a_new = _a;
			int b_new = _b;
			if(i < _a)
				{ a_new = i; }
			if(i >= _b)
				{ b_new = i + 1; }
			resize(a_new,b_new);
			}
//! @brief Minimum and maximum values ever set with resize() since object was created.
		int a() const
			{ return _a; }
		int b() const
			{ return _b; }
//! @brief Since resize() will move elements around, the access methods are fast.
		const T& operator[](int i) const
			{ return _val[i - _a]; }
		T& operator[](int i)
			{ return _val[i - _a]; }
		const T& at(int i) const
// glibc-problem, see comment at top of file.
//			{ return _val.at(i - _a); }
			{ return _val[i - _a]; }
		T& at(int i)
// glibc-problem, see comment at top of file.
//			{ return _val.at(i - _a); }
			{ return _val[i - _a]; }
//! @brief Iterate over the entire container, not just [a,b[.
		const_iterator begin() const
			{ return _val.begin(); }
		const_iterator end() const
			{ return _val.end(); }
		bool empty() const
			{ return _val.empty(); }
//! @brief Entire size, not just [a,b[.
		int size() const
			{ return _val.size(); }
		};
	template <class T>
	class cache_line_buffer:public extendible_vector<T>
		{
	public:
		typedef cache_line_buffer this_type;
		typedef extendible_vector<T> base_type;
	private:
	public:
		cache_line_buffer()
			{ }
		cache_line_buffer(const this_type& c):base_type(c)
			{ }
		};

//! @brief In Multithreading, more than one thread may work on the same line
//! reading and writing, however resizing is not allowed.
	template <class T>
	class line_ref
		{
	public:
		typedef cache_line_buffer<T> cache_line_buffer_type;
//	private:
		cache_line_buffer_type *_buffer;
	public:
		line_ref(cache_line_buffer_type& b):_buffer(&b)
			{ }
		int a() const
			{ return _buffer->a(); }
		int b() const
			{ return _buffer->b(); }
		const T& operator[](int i) const
			{ return (*_buffer)[i]; }
		T& operator[](int i)
			{ return (*_buffer)[i]; }
		bool exists(int i) const
			{ return (a() <= i) && (i < b()); }
//! @brief Exists or not?
		operator bool() const
			{ return _buffer; }
		};

//! @brief The class is thread-safe in the following sense:
//! - One thread operates on one line.
//! - Lines can be inserted one-by-one, but only deleted all at once.
//! - Allocating memory for a line is done at once and cannot be altered afterwards.
	template <class T>
	class line_cache
		{
	public:
		typedef line_cache this_type;
		typedef line_ref<T> line_ref_type;
		typedef cache_line_buffer<T> cache_line_buffer_type;
	private:
#ifdef _WINDOWS
		mutable CRITICAL_SECTION _critsec;
#else
		mutable pthread_mutex_t _mutex;
#endif
		typedef std::map<int,cache_line_buffer_type> buffers_type;
		buffers_type _buffers;
// Fast access
		typedef extendible_vector<cache_line_buffer_type*> buffer_refs_type;
		buffer_refs_type _buffer_refs;
	public:
		line_cache()
			{
#ifdef _WINDOWS
			InitializeCriticalSection(&_critsec);
#else
			int r = pthread_mutex_init(&_mutex,NULL);
			if(r)
				{ std::cerr << "ldpk::ldp_builtin::pthread_mutex_init: " << strerror(r) << std::endl; }
#endif
			}
		~line_cache()
			{
#ifdef _WINDOWS
			DeleteCriticalSection(&_critsec);
#else
			int r = pthread_mutex_destroy(&_mutex);
			if(r)
				{ std::cerr << "ldpk::ldp_builtin::pthread_mutex_destroy: " << strerror(r) << std::endl; }
#endif
			}
		void clear()
			{
			_buffers.clear();
			_buffer_refs.clear();
			}
		bool line_exists(int iy) const
			{
// glibc-problem, see comment at top of file.
//			return _buffer_refs.at(iy);
			return _buffer_refs[iy];
			}
//! @brief Protected by mutex. By this method, line iy is created -if not exists- and initialized.
		line_ref_type resize_line(int iy,int a,int b)
			{
#ifdef _WINDOWS
			EnterCriticalSection(&_critsec);
#else
			pthread_mutex_lock(&_mutex);
#endif
			try
				{
// Create buffer if not exists.
				cache_line_buffer_type& buffer = static_cast<cache_line_buffer_type&>(_buffers[iy].resize(a,b));
// Insert into array for fast access.
				_buffer_refs.extend(iy);
// glibc-problem, see comment at top of file.
//				_buffer_refs.at(iy) = &buffer;
				_buffer_refs[iy] = &buffer;
// Done. Return line reference, so threads can work on buffer.
#ifdef _WINDOWS
				LeaveCriticalSection(&_critsec);
#else
				pthread_mutex_unlock(&_mutex);
#endif
				return line_ref_type(buffer);
				}
			catch(const std::exception& e)
				{
				std::cerr << "ldpk::line_cache::resize_line(" << iy << "," << a << "," << b << ") failed: " << e.what() << std::endl;
#ifdef _WINDOWS
				LeaveCriticalSection(&_critsec);
#else
				pthread_mutex_unlock(&_mutex);
#endif
				throw;
				}
			catch(...)
				{
				std::cerr << "ldpk::line_cache::resize_line(" << iy << "," << a << "," << b << ") failed: unknown exception" << std::endl;
#ifdef _WINDOWS
				LeaveCriticalSection(&_critsec);
#else
				pthread_mutex_unlock(&_mutex);
#endif
				throw;
				}
			}
//! @brief Return reference to line_buffer if exists, otherwise exception.
		line_ref_type get_line(int iy) const
			{
#ifdef _WINDOWS
			EnterCriticalSection(&_critsec);
#else
			pthread_mutex_lock(&_mutex);
#endif
			try
				{
// Fast access.
// glibc-problem, see comment at top of file.
//				cache_line_buffer_type *buffer = _buffer_refs.at(iy);
				cache_line_buffer_type *buffer = _buffer_refs[iy];
				if(!buffer)
					{ throw std::range_error("ldpk::line_cache::get_line"); }
#ifdef _WINDOWS
				LeaveCriticalSection(&_critsec);
#else
				pthread_mutex_unlock(&_mutex);
#endif
				return line_ref_type(*buffer);
				}
			catch(const std::exception& e)
				{
				std::cerr << "ldpk::line_cache::get_line(" << iy << ") failed: " << e.what() << std::endl;
#ifdef _WINDOWS
				LeaveCriticalSection(&_critsec);
#else
				pthread_mutex_unlock(&_mutex);
#endif
				throw;
				}
			catch(...)
				{
				std::cerr << "ldpk::line_cache::resize_line(" << iy << ") failed: unknown exception" << std::endl;
#ifdef _WINDOWS
				LeaveCriticalSection(&_critsec);
#else
				pthread_mutex_unlock(&_mutex);
#endif
				throw;
				}
			}
//! @brief Number of Buffer-Refs.
		int size() const
			{ return _buffer_refs.size(); }
		friend std::ostream& operator<<(std::ostream& cout,const this_type& c)
			{
			for(int i = c._buffer_refs.a();i < c._buffer_refs.b();++i)
				{
				if(c.line_exists(i))
					{
					line_ref_type l = c.get_line(i);
					cout << "line " << i << ": " << l.a() << "," << l.b() << std::endl;
					}
				}
			return cout;
			}
		};
	}

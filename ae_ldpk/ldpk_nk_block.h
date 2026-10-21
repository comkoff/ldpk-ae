// LDPK-to-AfterEffects bridge.
// Parser for LD_3DE4 lens distortion node blocks as written by 3DE4's
// "Export Nuke LD_3DE4 Lens Distortion Node" script
// (3de4/python/export_nuke_LD_3DE4_Lens_Distortion_Node.py).
//
// This file is original code for the AE bridge; it follows the file format
// documented by the LDPK Nuke plugins (nuke_ld_3de4_base.C).
// Software is provided "as is" - no warranties implied.

#pragma once

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <istream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ae_ldpk
	{
//! @brief One parameter value: either static or animated (Nuke curve with keys).
	struct nk_value
		{
		bool animated;
		double static_value;
		std::vector<std::pair<int,double> > keys; // (frame,value), sorted by frame

		nk_value():animated(false),static_value(0.0)
			{ }
		void add_key(int frame,double v)
			{
			animated = true;
			keys.push_back(std::make_pair(frame,v));
			}
		void sort_keys()
			{
// Insertion sort, key lists are short.
			for(size_t i = 1;i < keys.size();++i)
				{
				std::pair<int,double> k = keys[i];
				size_t j = i;
				while((j > 0) && (keys[j - 1].first > k.first))
					{ keys[j] = keys[j - 1];--j; }
				keys[j] = k;
				}
			}
//! @brief Linear interpolation between keys, clamped at both ends.
//! (The 3DE4 exporter writes plain key lists; Nuke's default interpolation
//! for scripted curves is linear. Non-linear segments are not supported.)
		double eval(int frame) const
			{
			if(!animated)
				{ return static_value; }
			if(keys.empty())
				{ return 0.0; }
			if(frame <= keys.front().first)
				{ return keys.front().second; }
			if(frame >= keys.back().first)
				{ return keys.back().second; }
			for(size_t i = 1;i < keys.size();++i)
				{
				if(frame <= keys[i].first)
					{
					int f0 = keys[i - 1].first;
					int f1 = keys[i].first;
					double v0 = keys[i - 1].second;
					double v1 = keys[i].second;
					if(f1 == f0)
						{ return v1; }
					double q = double(frame - f0) / double(f1 - f0);
					return v0 + q * (v1 - v0);
					}
				}
			return keys.back().second;
			}
		};

//! @brief Binary serialization helpers for the sequence-data snapshot
//! (AE bridge). Host byte order, no alignment: every multi-byte
//! access goes through memcpy or explicit byte assembly, so the blob may
//! start at any alignment inside the host handle.
	namespace nk_ser
		{
		inline void put_u8(std::vector<unsigned char>& out,unsigned char v)
			{ out.push_back(v); }
		inline void put_u32(std::vector<unsigned char>& out,unsigned int v)
			{
			out.push_back((unsigned char)(v & 0xFF));
			out.push_back((unsigned char)((v >> 8) & 0xFF));
			out.push_back((unsigned char)((v >> 16) & 0xFF));
			out.push_back((unsigned char)((v >> 24) & 0xFF));
			}
		inline void put_i32(std::vector<unsigned char>& out,int v)
			{ put_u32(out,(unsigned int)v); }
		inline void put_f64(std::vector<unsigned char>& out,double v)
			{
			unsigned char b[8];
			memcpy(b,&v,8);
			out.insert(out.end(),b,b + 8);
			}
		inline void put_str(std::vector<unsigned char>& out,const std::string& s)
			{
			put_u32(out,(unsigned int)s.size());
			out.insert(out.end(),s.begin(),s.end());
			}
//! @brief Bounds-checked reader; ok goes false on the first overrun.
		struct reader
			{
			const unsigned char* p;
			size_t n;
			size_t pos;
			bool ok;
			reader(const unsigned char* data,size_t size):p(data),n(size),pos(0),ok(true)
				{ }
			bool get_u8(unsigned char& v)
				{
				if(!ok || (pos + 1 > n))
					{ ok = false;return false; }
				v = p[pos++];
				return true;
				}
			bool get_u32(unsigned int& v)
				{
				if(!ok || (pos + 4 > n))
					{ ok = false;return false; }
				v =	(unsigned int)p[pos] |
					((unsigned int)p[pos + 1] << 8) |
					((unsigned int)p[pos + 2] << 16) |
					((unsigned int)p[pos + 3] << 24);
				pos += 4;
				return true;
				}
			bool get_i32(int& v)
				{
				unsigned int u;
				if(!get_u32(u))
					{ return false; }
				v = (int)u;
				return true;
				}
			bool get_f64(double& v)
				{
				if(!ok || (pos + 8 > n))
					{ ok = false;return false; }
				memcpy(&v,p + pos,8);
				pos += 8;
				return true;
				}
			bool get_str(std::string& s)
				{
				unsigned int len;
				if(!get_u32(len))
					{ return false; }
				if(pos + (size_t)len > n)
					{ ok = false;return false; }
				s.assign((const char*)p + pos,(size_t)len);
				pos += len;
				return true;
				}
			};
		}

//! @brief Tokenizer for the small subset of the Nuke script language
//! we need: identifiers, numbers, braces, and '#' comments.
	class nk_lexer
		{
		private:
			std::istream& _in;
		public:
			nk_lexer(std::istream& in):_in(in)
				{ }
			bool next(std::string& s)
				{
				for(;;)
					{
					s = "";
					char c;
					bool got = false;
					while(_in.get(c))
						{
						if(!isspace((unsigned char)c))
							{ got = true;break; }
						}
					if(!got)
						{ return false; }
					if(c == '#')
						{
// Comment: skip to end of line.
						while(_in.get(c))
							{
							if(c == '\n')
								{ break; }
							}
						continue;
						}
					if((c == '{') || (c == '}'))
						{ s += c;return true; }
					s += c;
					while(_in.get(c))
						{
						if(isspace((unsigned char)c))
							{ break; }
						if((c == '{') || (c == '}') || (c == '#'))
							{ _in.putback(c);break; }
						s += c;
						}
					return true;
					}
				}
		};

//! @brief One LD_3DE4 node block from a .nk file.
	struct nk_ld_node
		{
		std::string nuke_class;	// e.g. "LD_3DE4_Brown_Conrady_Degree_6_2"
		std::string name;	// e.g. "LD_3DE4_grid_data_..."
		std::string direction;	// "distort" or "undistort"
		std::map<std::string,nk_value> params;

		const nk_value* find(const std::string& key) const
			{
			std::map<std::string,nk_value>::const_iterator i = params.find(key);
			return (i == params.end()) ? 0 : &i->second;
			}
		double get_double(const std::string& key,double default_value,int frame) const
			{
			const nk_value* v = find(key);
			return v ? v->eval(frame) : default_value;
			}
		bool parse(std::istream& in,std::string& err);

//! @brief Binary serialization of the parsed node (AE bridge):
//! the AE plugin stores this blob in its sequence data, so the project is
//! self-sufficient and renders never touch the .nk file. Layout (host
//! byte order, no alignment):
//!   string nuke_class, string name, string direction
//!       (each string: u32 length + bytes)
//!   u32 parameter count
//!   per parameter: string name, u8 animated, f64 static_value,
//!                  u32 key count, keys: (i32 frame, f64 value)
//! deserialize() validates every length against the buffer
//! size and rejects absurd counts.
		bool serialize(std::vector<unsigned char>& out,std::string& err) const;
		bool deserialize(const unsigned char* data,size_t size,std::string& err);
		};

	inline bool nk_ld_node::parse(std::istream& in,std::string& err)
		{
		nk_lexer lex(in);
		std::string t;
		if(!lex.next(t))
			{ err = "unexpected end of file";return false; }
		nuke_class = t;
		if(!lex.next(t) || (t != "{"))
			{ err = "expected '{' after class name";return false; }
		while(lex.next(t))
			{
			if(t == "}")
				{ return true; }
			std::string key = t;
			if(!lex.next(t))
				{ err = "unexpected end of file in node body";return false; }
			if(t == "{")
				{
// Animated value: {{curve x<frame> <value> ... }}
				std::string t2;
				if(!lex.next(t2) || (t2 != "{"))
					{ err = "expected '{{' starting curve for '" + key + "'";return false; }
				if(!lex.next(t2) || (t2 != "curve"))
					{ err = "expected 'curve' for '" + key + "'";return false; }
				nk_value v;
				v.animated = true;
				while(lex.next(t2))
					{
					if(t2 == "}")
						{
						std::string t3;
						if(!lex.next(t3) || (t3 != "}"))
							{ err = "expected '}}' ending curve for '" + key + "'";return false; }
						break;
						}
					if((t2[0] == 'x') && (t2.size() > 1))
						{
						int frame = atoi(t2.c_str() + 1);
						std::string tv;
						if(!lex.next(tv))
							{ err = "missing value in curve for '" + key + "'";return false; }
						v.add_key(frame,atof(tv.c_str()));
						}
					else
						{ err = "unexpected token in curve for '" + key + "'";return false; }
					}
				v.sort_keys();
				params[key] = v;
				}
			else
				{
				nk_value v;
				v.static_value = atof(t.c_str());
				params[key] = v;
				if(key == "name")
					{ name = t; }
				else if(key == "direction")
					{ direction = t; }
				}
			}
		err = "unexpected end of file, missing '}'";
		return false;
		}

	inline bool nk_ld_node::serialize(std::vector<unsigned char>& out,std::string& err) const
		{
		out.clear();
		try
			{
			nk_ser::put_str(out,nuke_class);
			nk_ser::put_str(out,name);
			nk_ser::put_str(out,direction);
			nk_ser::put_u32(out,(unsigned int)params.size());
			for(std::map<std::string,nk_value>::const_iterator i = params.begin();i != params.end();++i)
				{
				nk_ser::put_str(out,i->first);
				nk_ser::put_u8(out,i->second.animated ? 1 : 0);
				nk_ser::put_f64(out,i->second.static_value);
				nk_ser::put_u32(out,(unsigned int)i->second.keys.size());
				for(size_t k = 0;k < i->second.keys.size();++k)
					{
					nk_ser::put_i32(out,i->second.keys[k].first);
					nk_ser::put_f64(out,i->second.keys[k].second);
					}
				}
			}
		catch(...)
			{ err = "out of memory while serializing the node";return false; }
		return true;
		}

	inline bool nk_ld_node::deserialize(const unsigned char* data,size_t size,std::string& err)
		{
		nk_ser::reader r(data,size);
		std::string cls,nm,dir;
		if(!r.get_str(cls) || !r.get_str(nm) || !r.get_str(dir))
			{ err = "truncated snapshot";return false; }
		unsigned int count;
		if(!r.get_u32(count))
			{ err = "truncated snapshot";return false; }
// Sanity limit: the largest model has 24 parameters.
		if(count > 4096u)
			{ err = "corrupt snapshot";return false; }
		std::map<std::string,nk_value> parsed;
		for(unsigned int i = 0;i < count;++i)
			{
			std::string key;
			unsigned char animated;
			double sv;
			unsigned int num_keys;
			if(!r.get_str(key) || !r.get_u8(animated) || !r.get_f64(sv) || !r.get_u32(num_keys))
				{ err = "truncated snapshot";return false; }
// Sanity limit: curves with more than a million keys are corrupt.
			if(num_keys > (1u << 20))
				{ err = "corrupt snapshot";return false; }
			nk_value v;
			v.animated = (animated != 0);
			v.static_value = sv;
			for(unsigned int k = 0;k < num_keys;++k)
				{
				int f;
				double val;
				if(!r.get_i32(f) || !r.get_f64(val))
					{ err = "truncated snapshot";return false; }
				v.keys.push_back(std::make_pair(f,val));
				}
			parsed[key] = v;
			}
		nuke_class = cls;
		name = nm;
		direction = dir;
		params.swap(parsed);
		return true;
		}
	}

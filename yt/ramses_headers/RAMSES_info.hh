/*
		This file is part of libRAMSES++ 
			a C++ library to access snapshot files 
			generated by the simulation code RAMSES by R. Teyssier
		
    Copyright (C) 2008-09  Oliver Hahn, ojha@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __READ_INFO_HH
#define __READ_INFO_HH

#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <cmath>

namespace RAMSES{

#define btest(var,pos) ((var) & (1<<(pos)))

//! the file format version of RAMSES
enum codeversion{
	version1, version2, version3
};

const int state_diagram[12][2][8] = 
{ 
	{ { 1, 2, 3, 2, 4, 5, 3, 5}, { 0, 1, 3, 2, 7, 6, 4, 5} },
	{ { 2, 6, 0, 7, 8, 8, 0, 7}, { 0, 7, 1, 6, 3, 4, 2, 5} },
	{ { 0, 9,10, 9, 1, 1,11,11}, { 0, 3, 7, 4, 1, 2, 6, 5} },
	{ { 6, 0, 6,11, 9, 0, 9, 8}, { 2, 3, 1, 0, 5, 4, 6, 7} },
	{ {11,11, 0, 7, 5, 9, 0, 7}, { 4, 3, 5, 2, 7, 0, 6, 1} },
	{ { 4, 4, 8, 8, 0, 6,10, 6}, { 6, 5, 1, 2, 7, 4, 0, 3} },
	{ { 5, 7, 5, 3, 1, 1,11,11}, { 4, 7, 3, 0, 5, 6, 2, 1} },
  {	{ 6, 1, 6,10, 9, 4, 9,10}, { 6, 7, 5, 4, 1, 0, 2, 3} },
  { {10, 3, 1, 1,10, 3, 5, 9}, { 2, 5, 3, 4, 1, 6, 0, 7} },
  { {	4, 4, 8, 8, 2, 7, 2, 3}, { 2, 1, 5, 6, 3, 0, 4, 7} },
  { {	7, 2,11, 2, 7, 5, 8, 5}, { 4, 5, 7, 6, 3, 2, 0, 1} },
	{ {10, 3, 2, 6,10, 3, 4, 4}, { 6, 1, 7, 0, 5, 2, 4, 3} }
};
	
/**************************************************************************************\
\**************************************************************************************/
	
/*!
 * @class RAMSES_snapshot
 * @brief class providing access to general information about a RAMSES simulation snapshot
 */
class snapshot{

protected:

	//! c++ input file stream for stream access to the RAMSES info_XXXXX.txt file
	std::ifstream m_ifs;
	
public:

	//! path and name of the info_XXXXX.txt file of a RAMSES snapshot
	std::string   m_filename;
	
	//! the RAMSES version, takes care of changes in file format
	codeversion m_version;
	
	//! the simulation meta data stored in the info file
	struct info_data{
		unsigned ncpu;			//!< the number of CPUs (and thus computational domains) in this simulation
		unsigned ndim;			//!< the number of spatial dimensions
		unsigned levelmin;		//!< minimum refinement level (this refinement is thus present everywhere)
		unsigned levelmax;		//!< maximum refinement level allowed
		unsigned ngridmax;		//!< maximum number of grid cells stored per CPU
		unsigned nstep_coarse;	//!< number of coarse time steps performed up to this snapshot
		double boxlen;			//!< the length of the simulation box in internal units
		double time;			//!< time stamp of the snapshot
		double aexp;			//!< cosmological expansion factor of the current snapshot
		double H0;				//!< value of the Hubble constant for this simulation
		double omega_m;			//!< value of the total matter density parameter for this simulation
		double omega_l;			//!< value of the cosmological constant density parameter for this simulation
		double omega_k;			//!< value of the curvature density parameter for this simulation
		double omega_b;			//!< value of the baryonic (i.e. collisional) matter density parameter for this simulation
		double unit_l;			//!< internal length unit 
		double unit_d;			//!< internal density unit
		double unit_t;			//!< internal time unit
	};
	
	//! header data entailing time stamp and parameters of the simulation snapshot
	struct info_data m_header;
	
	//! minimum hilbert ordering indices for each domain
	std::vector<double> ind_min;
	
	//! maximum hilbert ordering indices for each domain
	std::vector<double> ind_max;
	
protected:
	
	//! tokenize a string 
	/*!
	 * @param str input string
	 * @param tokens reference to an STL vector containing all substrings after return
	 */
	void tokenize( const std::string& str, std::vector<std::string>& tokens )
	{
		std::string buf;
    	std::stringstream ss(str);
		while( ss >> buf )
			tokens.push_back( buf );
	}
	
	//! templated type conversion for string to arbitrary types that can be read from STL stringstreams
	/*!
	 * @param str input string
	 * @param ret reference to the template parameter type return value
	 */
	template <typename T>
	inline void convert( const std::string& str, T& ret )
	{
		std::stringstream ss(str);
		ss >> ret;
	}
	
	//! reads the right-most substring of a line
	/*!
	 * &param rhs reference to return value
	 */
	template <typename T>
	void read_line_rhs( T& rhs )
	{
		std::vector<std::string> split_buf;
		std::string buf;
		std::getline( m_ifs, buf );
		
		tokenize( buf, split_buf );
		convert( split_buf[split_buf.size()-1], rhs );
	}
	
	//! skip a line in the text input file
	void skip_line( void )
	{
		std::string buf;
		std::getline( m_ifs, buf );
	}
	
	//! parse the RAMSES snapshot info file
	void parse_file( void )
	{
		read_line_rhs( m_header.ncpu );
		read_line_rhs( m_header.ndim );
		read_line_rhs( m_header.levelmin );
		read_line_rhs( m_header.levelmax );
		read_line_rhs( m_header.ngridmax );
		read_line_rhs( m_header.nstep_coarse );
		skip_line();
		read_line_rhs( m_header.boxlen );
		read_line_rhs( m_header.time );
		read_line_rhs( m_header.aexp );
		read_line_rhs( m_header.H0 );
		read_line_rhs( m_header.omega_m );
		read_line_rhs( m_header.omega_l );
		read_line_rhs( m_header.omega_k );
		read_line_rhs( m_header.omega_b );
		read_line_rhs( m_header.unit_l );
		read_line_rhs( m_header.unit_d );
		read_line_rhs( m_header.unit_t );
		skip_line();
		skip_line();
		skip_line();
		
		for( unsigned i=0; i<m_header.ncpu; ++i ){
			std::vector<std::string> split_buf;
			std::string buf;
			std::getline( m_ifs, buf );
			tokenize( buf, split_buf );
			
			double imin, imax;
			unsigned domain;
			
			convert( split_buf[0], domain );
			
			if( domain != i+1 )
				throw std::runtime_error("RAMSES_snapshot::parse_file : corrupt info file\'"+m_filename+"\'.");
			
			convert( split_buf[1], imin );
			convert( split_buf[2], imax );
			
			ind_min.push_back( imin );
			ind_max.push_back( imax );
			
		}
		
		
	}
	
	inline unsigned locate( const double x, const std::vector<double>& vx )
	{
		long unsigned ju,jm,jl;
		bool ascnd=(vx[vx.size()-1]>=vx[0]);
		jl = 0;
		ju = vx.size()-1;
		while( ju-jl > 1 ) {
			jm = (ju+jl)>>1;
			if( x >= vx[jm] == ascnd )
				jl = jm;
			else
				ju = jm;
		}
		return std::max((long unsigned)0,std::min((long unsigned)(vx.size()-2),(long unsigned)jl));
	}
	
public:
	
	//! constructor for a RAMSES snapshot meta data object
	/*!
	 * @param info_filename path and name of the info_XXXXX.txt file of the RAMSES simulation
	 */
	snapshot( std::string info_filename, codeversion ver=RAMSES::version1 )
	 : m_ifs( info_filename.c_str(), std::ios::in ), m_filename( info_filename ),
	   m_version( ver )
	{
		if(!m_ifs.good())
			throw std::runtime_error("RAMSES_snapshot : cannot open file \'"+m_filename+"\' for read access.");
		m_ifs.exceptions ( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
		
		parse_file();
	}
	

	unsigned get_snapshot_num( void )
	{
		unsigned ii = m_filename.rfind("info"), num;
		std::string tmp( m_filename.substr(ii+5,5) );
		sscanf(tmp.c_str(),"%d",&num);
		return num;
	}
	
	unsigned getdomain_bykey( double key )
	{
		unsigned i = locate( key, ind_min );
		return i+1;
	}


};


inline void hilbert3d( 
		const std::vector<double>& x,
		const std::vector<double>& y,
		const std::vector<double>& z,
		std::vector<double>& order,
		unsigned bit_length )
{
	std::vector<bool> i_bit_mask(3*bit_length, false );
	std::vector<bool> x_bit_mask(1*bit_length, false );
	std::vector<bool> y_bit_mask(1*bit_length, false );
	std::vector<bool> z_bit_mask(1*bit_length, false );
	
	unsigned npoints = x.size();
	double lx = pow(2.0,bit_length);
	
	for( unsigned ip=0; ip<npoints; ++ip )
	{
		for( unsigned i=0; i<bit_length; ++i )
		{
			int xx = (int)(x[ip]*lx);
			x_bit_mask[i] = btest(xx,i);
			
			xx = (int)(y[ip]*lx);
			y_bit_mask[i] = btest(xx,i);
			
			xx = (int)(z[ip]*lx);
			z_bit_mask[i] = btest(xx,i);
		}
		
		for( unsigned i=0; i<bit_length; ++i )
		{
			i_bit_mask[3*i+2] = x_bit_mask[i];
			i_bit_mask[3*i+1] = y_bit_mask[i];
			i_bit_mask[3*i+0] = z_bit_mask[i];
		}
		
		int nstate, cstate;
		bool b0, b1, b2;
		int sdigit, hdigit;
		
		cstate=0;
		for( int i=bit_length-1; i>=0; --i )
		{
			b2 = i_bit_mask[3*i+2];
			b1 = i_bit_mask[3*i+1];
			b0 = i_bit_mask[3*i+0];
			
			sdigit = b2*4+b1*2+b0;
			nstate = state_diagram[cstate][0][sdigit];
			hdigit = state_diagram[cstate][1][sdigit];
			
			i_bit_mask[3*i+2] = btest(hdigit,2);
			i_bit_mask[3*i+1] = btest(hdigit,1);
			i_bit_mask[3*i+0] = btest(hdigit,0);
			
			cstate = nstate;
		}
		
		order[ip] = 0.0;
		for( unsigned i=0; i<3*bit_length; ++i )
		{
			b0 = i_bit_mask[i];
			order[ip] += (double)b0*pow(2.0,i);
		}
		
	}	
}


#undef btest

}


#endif

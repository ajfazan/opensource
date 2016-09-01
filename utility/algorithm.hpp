
#ifndef UTILITY_ALGORITHM_HPP
#define UTILITY_ALGORITHM_HPP

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>

namespace utility {

  namespace algorithm {

    template <class container_type>
    boost::shared_ptr<container_type> load( const std::string& filename )
    {
      typedef typename container_type::value_type data_type;

      std::ifstream in( filename.c_str() );
      BOOST_ASSERT( in.is_open() );
      std::istream_iterator<data_type> begin( in ), end;
      boost::shared_ptr<container_type> data(
        new container_type( begin, end )
      );
      in.close();
      return data;
    }

    template <class container_type>
    void write( const std::string& filename,
                const container_type& container,
                const std::ios_base::fmtflags& format,
                const size_t& p = 6 )
    {
      typedef typename container_type::value_type data_type;

      std::ofstream out( filename.c_str() );
      out.flags( format );
      out.precision( p );
      std::ostream_iterator<data_type> out_it( out, "\n" );
      std::copy( container.begin(), container.end(), out_it );
      out.close();
    }

  }

}

#endif

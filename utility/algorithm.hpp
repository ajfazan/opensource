
#ifndef UTILITY_ALGORITHM_HPP
#define UTILITY_ALGORITHM_HPP

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>

#include <ifstream>
#include <iterator>

namespace utility {

  namespace algorithm {

    template <class container_type>
    boost::shared_ptr<container_type> load( const std::string& filename )
    {
      std::ifstream in( filename.c_str() );
      BOOST_ASSERT( in.is_open() );
      std::istream_iterator<container_type::value_type> begin( in ), end;
      boost::shared_ptr<container_type> data( new container_type );
      data->insert( begin, end );
      in.close();
      return data;
    }

  }

}

#endif

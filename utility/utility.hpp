
#ifndef UTILITY_UTILITY_HPP
#define UTILITY_UTILITY_HPP

#include <boost/assert.hpp>
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <string>

namespace utility {

  template <typename target_type>
  target_type from_string( const std::string& num )
  {
    target_type result( 0 );

    try {

      result = boost::lexical_cast<target_type>( num );

    } catch( boost::bad_lexical_cast& e ) {

      std::cerr << e.what() << std::endl;
    }

    return result;
  }

  template <typename target_type, typename source_type>
  target_type cast( const source_type& num )
  {
    target_type result( 0 );

    try {

      result = boost::numeric_cast<target_type>( num );

    } catch( boost::bad_numeric_cast& e ) {

      std::cerr << e.what() << std::endl;
    }

    return result;
  }

}

#endif

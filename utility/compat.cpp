#include <utility/compat.hpp>


#include <string>

namespace utility {

  struct tm strptime( const char* timestamp )
  {
    std::string ts( timestamp );
    boost::posix_time::ptime ptime = boost::posix_time::time_from_string( ts );
    return boost::posix_time::to_tm( ptime );
  }

}

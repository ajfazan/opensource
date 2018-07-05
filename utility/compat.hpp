
#ifndef UTILITY_COMPAT_HPP
#define UTILITY_COMPAT_HPP

#include <boost/date_time/posix_time/posix_time.hpp>

namespace utility {

  struct tm strptime( const char* timestamp );

}

#endif

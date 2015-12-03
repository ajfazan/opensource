
#ifndef UTILITY_MAPPED_MEMORY_HPP
#define UTILITY_MAPPED_MEMORY_HPP

#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>

#include <cstdio>
#include <iostream>
#include <string>

namespace utility {

  template <class num_type>
  class mapped_memory {

  public:
    static const boost::uint64_t MAX_IN_MEMORY = 16777216;

    explicit mapped_memory( const boost::uint64_t& count = 0 )
      : ptr_( 0 ), count_( count ), fd_( -1 )
    {
      reserve();
    }

    mapped_memory( const mapped_memory& other )
      : ptr_( 0 ), count_( other.count_ ),
        fd_( other.fd_ ), path_( other.path_ )
    {
      memcpy( ptr_, other.ptr_, bytes() );
    }

    mapped_memory& operator=( const mapped_memory& other )
    {
      mapped_memory mm( other );
      swap( mm );
      return *this;
    }

    ~mapped_memory()
    {
      release();
    }

    operator bool() const
    {
      return ( ptr_ != 0 );
    }

    num_type& operator[]( const boost::uint64_t& index )
    {
      BOOST_ASSERT( ptr_ != 0 );
      BOOST_ASSERT( index < count_ );
      return ptr_[index];
    }

    const num_type& operator[]( const boost::uint64_t& index ) const
    {
      BOOST_ASSERT( ptr_ != 0 );
      BOOST_ASSERT( index < count_ );
      return ptr_[index];
    }

    num_type* get() const
    {
      return ptr_;
    }

    bool is_mapped() const
    {
      return ( count_ > MAX_IN_MEMORY );
    }

    boost::uint64_t size() const
    {
      return count_;
    }

    boost::uint64_t bytes() const
    {
      return count_ * sizeof( num_type );
    }

    void swap( mapped_memory& other )
    {
      std::swap( other.ptr_  , ptr_   );
      std::swap( other.count_, count_ );
      std::swap( other.fd_   , fd_    );
      std::swap( other.path_ , path_  );
    }

    void reserve()
    {
      BOOST_ASSERT( ptr_ == 0 );

      if( !count_ ) {

        return;
      }

      if( is_mapped() ) {

        boost::filesystem::path tmp( boost::filesystem::temp_directory_path() );
        filename_ = tmp.string() + "/mm_XXXXXX";

        char name[256];
        strcpy( name, filename_.c_str() );

        fd_ = mkstemp( name );
        filename_ = std::string( name );

        if( fd_ == -1 ) {

          std::cerr << "Unable to create map file: " << filename_ << std::endl;
          return;
        }

        off_t file_size( bytes() );
        ftruncate( fd_, file_size );

        void* mem = mmap( 0, file_size, PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_NORESERVE, fd_, 0 );

        BOOST_ASSERT( mem != MAP_FAILED );
        ptr_ = static_cast<num_type*>( mem );

      } else {

        ptr_ = new num_type[count_];
      }
    }

    void release()
    {
      if( !ptr_ ) {

        return;
      }

      if( is_mapped() ) {

        munmap( ptr_, bytes() );
        close( fd_ );

        boost::filesystem::remove(
          boost::filesystem::path( filename_.c_str() )
        );

      } else {

        delete[] ptr_;
      }

      ptr_ = 0;
    }

  private:
    num_type* ptr_;

    boost::uint64_t count_;

    std::string filename_;

    int fd_;

    std::string path_;

  };

}

#endif

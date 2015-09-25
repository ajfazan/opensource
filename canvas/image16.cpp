
#include <canvas/image16.hpp>

#include <boost/numeric/ublas/matrix.hpp>

#include <boost/assert.hpp>

namespace canvas {

  image16::image16( const size_t& lines,
                    const size_t& columns,
                    const size_t& channels ) : image( lines, columns, channels )
  {
    bands_.reserve( channels_ );
  }

  image16::image16( const std::string& filename ) : image( filename )
  {
  }

  image16::~image16()
  {
  }

  void image16::allocate()
  {
    if( bands_.empty() ) {

      boost::uint64_t pixels( lines_ * columns_ );

      for( size_t k = 0; k < channels_; ++k ) {

        bands_.push_back( band_ptr( new band( pixels ) ) );
      }
    }
  }

  void image16::load()
  {
    boost::uint64_t pixels( lines_ * columns_ );

    boost::uint16_t* buffer = static_cast<boost::uint16_t*>(
      CPLMalloc( pixels * sizeof( boost::uint16_t ) )
    );

    std::vector<band_ptr>::iterator b_it = bands_.begin();

    for( size_t k = 0; k < channels_; ++k, ++b_it ) {

      GDALRasterBand* b_handle = dataset_->GetRasterBand( k + 1 );

      size_t x_size( b_handle->GetXSize() );
      size_t y_size( b_handle->GetYSize() );

      BOOST_ASSERT( ( x_size * y_size ) == pixels );

      b_handle->RasterIO( GF_Read, 0, 0, x_size, y_size, buffer,
                                         x_size, y_size, GDT_UInt16, 0, 0 );

      std::copy( buffer, buffer + pixels, ( *b_it )->get() );
    }

    CPLFree( buffer );
  }

  boost::shared_array<double> image16::compute_values( const pixel& px ) const
  {
    boost::shared_array<double> g( new double[channels_] );
    const double* nodata_ptr( nodata_.get() );
    std::copy( nodata_ptr, nodata_ptr + channels_, g.get() );

    const double& x( px.get<0>() );
    const double& y( px.get<1>() );

    boost::uint64_t i( static_cast<boost::uint64_t>( y ) );
    boost::uint64_t j( static_cast<boost::uint64_t>( x ) );

    boost::uint64_t max_i( static_cast<boost::uint64_t>( lines_   ) - 1 );
    boost::uint64_t max_j( static_cast<boost::uint64_t>( columns_ ) - 1 );

    if( ( i <= max_i ) && ( j <= max_j ) ) {

      double dx( x - j );
      double dy( y - i );

      double cdx( 1.0 - dx );

      double* g_ptr( g.get() );

      for( size_t k = 1; k <= channels_; ++k, ++g_ptr ) {

        GDALRasterBand* b_handle = dataset_->GetRasterBand( k );

        float buffer[4];
        b_handle->RasterIO( GF_Read, j, i, 2, 2, buffer,
                                           2, 2, GDT_UInt16, 0, 0 );

        float nd( static_cast<float>( get_nodata( k ) ) );

        if( std::count( buffer, buffer + 4, nd ) == 0 ) {

          double ga( dx * buffer[1] + cdx * buffer[0] );
          double gb( dx * buffer[3] + cdx * buffer[2] );

          *g_ptr = dy * gb + ( 1.0 - dy ) * ga;
        }
      }
    }

    return g;
  }

  image16::band_ptr image16::get_band( size_t band_number ) const
  {
    BOOST_ASSERT( band_number >= 1 );
    BOOST_ASSERT( band_number <= channels_ );
    return bands_[band_number - 1];
  }

}
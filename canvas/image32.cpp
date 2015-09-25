
#include <canvas/image32.hpp>

#include <boost/assert.hpp>

namespace canvas {

  image32::image32( const size_t& lines,
                    const size_t& columns,
                    const size_t& channels ) : image( lines, columns, channels )
  {
    bands_.reserve( channels_ );
  }

  image32::image32( const std::string& filename ) : image( filename )
  {
  }

  image32::~image32()
  {
  }

  void image32::allocate()
  {
    if( bands_.empty() ) {

      boost::uint64_t pixels( lines_ * columns_ );

      for( size_t k = 0; k < channels_; ++k ) {

        bands_.push_back( band_ptr( new band( pixels ) ) );
      }
    }
  }

  void image32::load()
  {
    boost::uint64_t pixels( lines_ * columns_ );

    std::vector<band_ptr>::iterator b_it = bands_.begin();

    for( size_t k = 0; k < channels_; ++k, ++b_it ) {

      GDALRasterBand* b_handle = dataset_->GetRasterBand( k + 1 );

      size_t x_size( b_handle->GetXSize() );
      size_t y_size( b_handle->GetYSize() );

      BOOST_ASSERT( ( x_size * y_size ) == pixels );

      b_handle->RasterIO( GF_Read, 0, 0, x_size, y_size, ( *b_it )->get(),
                                         x_size, y_size, GDT_Float32, 0, 0 );
    }
  }

  image32::ptr image32::load( size_t l1, size_t c1, size_t l2, size_t c2 ) const
  {
    BOOST_ASSERT( l1 < l2 );
    BOOST_ASSERT( c1 < c2 );

    size_t lines  ( l2 - l1 );
    size_t columns( c2 - c1 );

    BOOST_ASSERT( lines   <= lines_   );
    BOOST_ASSERT( columns <= columns_ );

    image32::ptr region( new image32( lines, columns, channels_ ) );
    region->allocate();

    for( size_t k = 0; k < channels_; ++k ) {

      GDALRasterBand* b_handle = dataset_->GetRasterBand( k + 1 );

      band_ptr b_ptr( region->get_band( k + 1 ) );
      b_handle->RasterIO( GF_Read, c1, l1, columns, lines, b_ptr->get(),
                                           columns, lines, GDT_Float32, 0, 0 );
    }

    return region;
  }

  boost::shared_array<double> image32::compute_values( const pixel& px ) const
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
                                           2, 2, GDT_Float32, 0, 0 );

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

  image32::band_ptr image32::get_band( size_t band_number ) const
  {
    BOOST_ASSERT( band_number >= 1 );
    BOOST_ASSERT( band_number <= channels_ );
    return bands_[band_number - 1];
  }

  image32::ptr image32::compute_difference( const image32& other ) const
  {
    image32::ptr result;

    boost::shared_ptr<metadata> t_md( this->get_metadata() );
    boost::shared_ptr<metadata> o_md( other.get_metadata() );

    if( t_md && o_md ) {

      const CGAL::Bbox_2& t_bb( t_md->get<1>() );
      const CGAL::Bbox_2& o_bb( o_md->get<1>() );

      if( ( t_md->get<0>() == o_md->get<0>() ) &&
            CGAL::do_overlap( t_bb, o_bb ) ) {

        std::vector<double> x;
        std::vector<double> y;

        x.reserve( 4 );
        y.reserve( 4 );

        std::sort( x.begin(), x.end() );
        std::sort( y.begin(), y.end() );

        CGAL::Bbox_2 i_bb( x[0], y[0], x[1], y[1] );

        Kernel::Point_2 ul( x[0], y[1] );
        Kernel::Point_2 lr( x[1], y[0] );

        pixel t_ul( this->compute_position( ul ) );
        pixel t_lr( this->compute_position( lr ) );
        pixel o_ul( other.compute_position( ul ) );
        pixel o_lr( other.compute_position( lr ) );

        size_t t_l1( std::floor( t_ul.get<1>() ) );
        size_t t_c1( std::floor( t_ul.get<0>() ) );

        size_t t_l2( std::ceil( t_lr.get<1>() ) );
        size_t t_c2( std::ceil( t_lr.get<0>() ) );

        size_t o_l1( std::floor( o_ul.get<1>() ) );
        size_t o_c1( std::floor( o_ul.get<0>() ) );

        size_t o_l2( std::ceil( o_lr.get<1>() ) );
        size_t o_c2( std::ceil( o_lr.get<0>() ) );

        size_t lines( t_l2 - t_l1 );
        BOOST_ASSERT( lines == ( o_l2 - o_l1 ) );

        size_t columns( t_c2 - t_c1 );
        BOOST_ASSERT( columns == ( o_c2 - o_c1 ) );

        image32::ptr r1( this->load( t_l1, t_c1, t_l2, t_c2 ) );
        image32::ptr r2( other.load( o_l1, o_c1, o_l2, o_c2 ) );

        result.reset( new image32( lines, columns, channels_ ) );
        result->allocate();

        boost::uint64_t pixels( lines * columns );

        for( size_t k = 1; k <= channels_; ++k ) {

          band_ptr r1_b( r1->get_band( k ) );
          band_ptr r2_b( r2->get_band( k ) );

          const float* r1_b_ptr( r1_b->get() );
          const float* r2_b_ptr( r2_b->get() );

          band_ptr r_b( result->get_band( k ) );
          float* r_b_ptr( r_b->get() );

          const double& r1_nd( r1->get_nodata( k ) );
          const double& r2_nd( r2->get_nodata( k ) );

          for( boost::uint64_t n = 0; n < pixels; ++n,
                                                  ++r1_b_ptr,
                                                  ++r2_b_ptr, ++r_b_ptr ) {

            *r_b_ptr = ( ( *r1_b_ptr != r1_nd ) &&
                         ( *r2_b_ptr != r2_nd ) ) ? *r1_b_ptr - *r2_b_ptr : 0.0;
          }
        }
      }
    }

    return result;
  }

  image32::stats image32::compute_stats() const
  {
    std::vector<double> minimum;
    minimum.reserve( 4 );

    std::vector<double> maximum;
    maximum.reserve( 4 );

    std::vector<double> mean;
    mean.reserve( 4 );

    std::vector<double> variance;
    variance.reserve( 4 );

    boost::uint64_t pixels( lines_ * columns_ );
    std::vector<band_ptr>::const_iterator b_it = bands_.begin();

    for( size_t k = 0; k < channels_; ++k, ++b_it ) {

      double sgg( 0.0 ), sg( 0.0 ), n( 0.0 );

      double l( std::numeric_limits<double>::max() );
      double u( std::numeric_limits<double>::min() );

      double nd( get_nodata( k + 1 ) );

      const float* px( ( *b_it )->get() );

      for( boost::uint64_t p = 0; p < pixels; ++p, ++px ) {

        double g( *px );

        if( g != nd ) {

          if( l > g ) {

            l = g;
          }

          if( u < g ) {

            u = g;
          }

          sgg += g * g;
          sg  += g;
          ++n;
        }
      }

      minimum.push_back( l );
      maximum.push_back( u );

      mean.push_back( sg/ n );

      variance.push_back( sgg - ( sg * mean.back() ) / ( n - 1.0 ) );
    }

    return stats( minimum, maximum, mean, variance );
  }

}


#include <canvas/image8.hpp>

#include <boost/assert.hpp>

namespace canvas {

  image8::image8( const size_t& lines,
                  const size_t& columns,
                  const size_t& channels ) : image( lines, columns, channels )
  {
    bands_.reserve( channels_ );
  }

  image8::image8( const std::string& filename ) : image( filename )
  {
  }

  image8::~image8()
  {
  }

  void image8::allocate( bool fill )
  {
    if( bands_.empty() || ( bands_.size() != channels_ ) ) {

      boost::uint64_t pixels( lines_ * columns_ );

      bands_.clear();

      if( fill ) {

        for( size_t k = 0; k < channels_; ++k ) {

          band_ptr array( new band( pixels ) );
          boost::uint8_t* data_ptr( array->get() );
          BOOST_ASSERT( data_ptr );
          std::fill_n( data_ptr, pixels, 0 );
          bands_.push_back( array );
        }

      } else {

        for( size_t k = 0; k < channels_; ++k ) {

          bands_.push_back( band_ptr( new band( pixels ) ) );
        }
      }
    }
  }

  void image8::load()
  {
    boost::uint64_t pixels( lines_ * columns_ );

    boost::uint8_t* buffer = static_cast<boost::uint8_t*>(
      CPLMalloc( pixels * sizeof( boost::uint8_t ) )
    );

    std::vector<band_ptr>::iterator b_it = bands_.begin();

    for( size_t k = 0; k < channels_; ++k, ++b_it ) {

      GDALRasterBand* b_handle = dataset_->GetRasterBand( k + 1 );

      size_t x_size( b_handle->GetXSize() );
      size_t y_size( b_handle->GetYSize() );

      BOOST_ASSERT( ( x_size * y_size ) == pixels );

      CPLErr e = b_handle->RasterIO( GF_Read,
        0, 0, x_size, y_size, buffer, x_size, y_size, GDT_Byte, 0, 0 );

      BOOST_ASSERT( e == CE_None );

      std::copy( buffer, buffer + pixels, ( *b_it )->get() );
    }

    CPLFree( buffer );
  }

  image8::ptr image8::load( size_t l1, size_t c1, size_t l2, size_t c2 ) const
  {
    BOOST_ASSERT( l1 < l2 );
    BOOST_ASSERT( c1 < c2 );

    size_t lines  ( l2 - l1 );
    size_t columns( c2 - c1 );

    BOOST_ASSERT( lines   <= lines_   );
    BOOST_ASSERT( columns <= columns_ );

    image8::ptr region( new image8( lines, columns, channels_ ) );
    region->allocate();

    for( size_t k = 0; k < channels_; ++k ) {

      GDALRasterBand* b_handle = dataset_->GetRasterBand( k + 1 );

      band_ptr b_ptr( region->get_band( k + 1 ) );

      CPLErr e = b_handle->RasterIO( GF_Read, c1, l1, columns, lines,
        b_ptr->get(), columns, lines, GDT_Float32, 0, 0 );

      BOOST_ASSERT( e == CE_None );
    }

    return region;
  }

  void image8::write( const std::string& filename )
  {
    if( dataset_ != NULL ) {

      GDALClose( dataset_ );
    }

    CPLErr e;

    boost::filesystem::path p( filename );
    std::string ext( p.extension().string() );

    GDALDriver* driver =
      GetGDALDriverManager()->GetDriverByName( driver_[ext].c_str() );

    dataset_ = driver->Create( filename.c_str(), columns_, lines_, channels_,
                               GDT_Float32, NULL );

    const CGAL::Bbox_2& bb( md_->get<1>() );
    double transform[] = { bb.xmin(), md_->get<0>(), 0.0,
                           bb.ymax(), 0.0, md_->get<0>() };

    e = dataset_->SetGeoTransform( transform );
    BOOST_ASSERT( e == CE_None );

    e = dataset_->SetProjection( md_->get<2>().c_str() );
    BOOST_ASSERT( e == CE_None );

    std::vector<band_ptr>::const_iterator b_it = bands_.begin();

    for( size_t k = 0; k < channels_; ++k, ++b_it ) {

      GDALRasterBand* b_handle = dataset_->GetRasterBand( k + 1 );

      b_handle->SetNoDataValue( nodata_[k] );

      e = b_handle->RasterIO( GF_Write, 0, 0, columns_, lines_,
            ( *b_it )->get(), columns_, lines_, GDT_Byte, 0, 0 );

      BOOST_ASSERT( e == CE_None );
    }
  }

  boost::shared_array<double> image8::compute_values( const pixel& px ) const
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
        CPLErr e = b_handle->RasterIO( GF_Read, j, i, 2, 2, buffer,
                                       2, 2, GDT_Byte, 0, 0 );

        BOOST_ASSERT( e == CE_None );

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

  image8::band_ptr image8::get_band( size_t band_number ) const
  {
    BOOST_ASSERT( band_number >= 1 );
    BOOST_ASSERT( band_number <= channels_ );
    return bands_[band_number - 1];
  }

  image8::ptr image8::compute_difference( const image8& other ) const
  {
    image8::ptr result;

    boost::shared_ptr<metadata> t_md( this->get_metadata() );
    boost::shared_ptr<metadata> o_md( other.get_metadata() );

    if( t_md && o_md ) {

      const CGAL::Bbox_2& t_bb( t_md->get<1>() );
      const CGAL::Bbox_2& o_bb( o_md->get<1>() );

      if( ( t_md->get<0>() == o_md->get<0>() ) &&
          ( t_md->get<2>() == o_md->get<2>() ) &&
            CGAL::do_overlap( t_bb, o_bb ) ) {

        double x[] = { t_bb.xmin(), t_bb.xmax(), o_bb.xmin(), o_bb.xmax() };
        double y[] = { t_bb.ymin(), t_bb.ymax(), o_bb.ymin(), o_bb.ymax() };

        std::sort( x, x + 4 );
        std::sort( y, y + 4 );

        CGAL::Bbox_2 i_bb( x[1], y[1], x[2], y[2] );

        Kernel::Point_2 ul( x[1], y[2] );
        Kernel::Point_2 lr( x[2], y[1] );

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

        image8::ptr r1( this->load( t_l1, t_c1, t_l2, t_c2 ) );
        image8::ptr r2( other.load( o_l1, o_c1, o_l2, o_c2 ) );

        result.reset( new image8( lines, columns, channels_ ) );
        result->allocate( true );

        boost::uint64_t pixels( lines * columns );

        for( size_t k = 1; k <= channels_; ++k ) {

          band_ptr r1_b( r1->get_band( k ) );
          band_ptr r2_b( r2->get_band( k ) );

          const boost::uint8_t* r1_b_ptr( r1_b->get() );
          const boost::uint8_t* r2_b_ptr( r2_b->get() );

          band_ptr r_b( result->get_band( k ) );
          boost::uint8_t* r_b_ptr( r_b->get() );

          const double& r1_nd( r1->get_nodata( k ) );
          const double& r2_nd( r2->get_nodata( k ) );

          for( boost::uint64_t n = 0; n < pixels; ++n,
                                                  ++r1_b_ptr,
                                                  ++r2_b_ptr, ++r_b_ptr ) {

            if( ( *r1_b_ptr != r1_nd ) && ( *r2_b_ptr != r2_nd ) ) {

              *r_b_ptr = abs( *r1_b_ptr - *r2_b_ptr );
            }
          }
        }
      }
    }

    return result;
  }

}

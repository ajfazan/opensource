
#include <canvas/image.hpp>

#include <boost/assert.hpp>

namespace canvas {

  image::image( const size_t& lines,
                const size_t& columns,
                const size_t& channels )
    : lines_( lines ), columns_( columns ), channels_( channels ), dataset_( 0 )
  {
    BOOST_ASSERT( channels_ > 0 );
    nodata_.reset( new double[channels_] );
    BOOST_ASSERT( nodata_ != 0 );
    std::fill_n( nodata_.get(), channels_, 0.0 );
  }

  image::image( const std::string& filename )
  {
    GDALAllRegister();

    dataset_ = ( GDALDataset* ) GDALOpen( filename.c_str(), GA_ReadOnly );

    if( dataset_ == NULL ) {

      std::cerr << "Unable to open image " << filename << std::endl;
      return;
    }

    lines_    = dataset_->GetRasterXSize();
    columns_  = dataset_->GetRasterYSize();
    channels_ = dataset_->GetRasterCount();

    nodata_.reset( new double[channels_] );
    double* nodata_ptr( nodata_.get() );
    BOOST_ASSERT( nodata_ptr != 0 );

    GDALRasterBand* band_handle;
    int fetch;

    for( size_t k = 1; k <= channels_; ++k, ++nodata_ptr ) {

       band_handle = dataset_->GetRasterBand( k );
       double null( band_handle->GetNoDataValue( &fetch ) );
       *nodata_ptr = fetch ? null : 0.0;
    }

    if( dataset_->GetProjectionRef() != NULL ) {

      double tmp[6];

      if( dataset_->GetGeoTransform( tmp ) == CE_None ) {

        double pixel_size( tmp[1] );
        CGAL::Bbox_2 bb( tmp[0], tmp[3] - pixel_size * lines_,
                         tmp[0] + pixel_size * columns_, tmp[3] );
        std::string proj_tag( dataset_->GetProjectionRef() );

        md_.reset( new metadata( pixel_size, bb, proj_tag ) );
      }
    }
  }

  image::~image()
  {
    if( dataset_ != NULL ) {

      GDALClose( dataset_ );
    }
  }

  const size_t& image::get_lines() const
  {
    return lines_;
  }

  const size_t& image::get_columns() const
  {
    return columns_;
  }

  const size_t& image::get_channels() const
  {
    return channels_;
  }

  const double& image::get_nodata( size_t band_number ) const
  {
    BOOST_ASSERT( band_number >= 1 );
    BOOST_ASSERT( band_number <= channels_ );
    return nodata_[band_number - 1];
  }

  boost::shared_ptr<image::metadata> image::get_metadata() const
  {
    return md_;
  }

  void image::display_info( const std::string& tag ) const
  {
    std::cout << tag << std::endl;
    std::cout << "Size (lines x columns x bands): "
              << lines_ << " x " << columns_ << " x "
              << channels_ << std::endl;

    if( md_ ) {

      std::cout << "Pixel size: "   << md_->get<0>() << std::endl;
      std::cout.precision( 4 );
      std::cout.flags( std::ios::fixed );
      std::cout << "Bounding box: " << md_->get<1>() << std::endl;
    }

    std::cout << std::endl;
  }

  bool image::contains( const Kernel::Point_2& p ) const
  {
    BOOST_ASSERT( md_ );

    const double& x( p.x() );
    const double& y( p.y() );

    const CGAL::Bbox_2& bb( md_->get<1>() );

    return ( ( x >= bb.xmin() )
          && ( x <= bb.xmax() )
          && ( y >= bb.ymin() )
          && ( y <= bb.ymax() ) );
  }

  bool image::intersects( const image& other ) const
  {
    boost::shared_ptr<metadata> md( other.get_metadata() );

    if( md_ && md ) {

      const CGAL::Bbox_2& bb1( md_->get<1>() );
      const CGAL::Bbox_2& bb2( md ->get<1>() );

      return (
        ( md_->get<0>() == md->get<0>() ) && CGAL::do_overlap( bb1, bb2 )
      );
    }

    return false;
  }

  image::pixel image::compute_position( const Kernel::Point_2& p ) const
  {
    BOOST_ASSERT( md_ );

    const double& x( p.x() );
    const double& y( p.y() );

    const double& pixel_size( md_->get<0>() );

    const CGAL::Bbox_2& bb( md_->get<1>() );

    return pixel( ( x - bb.xmin() ) / pixel_size,
                  ( bb.ymax() - y ) / pixel_size );
  }

  bool image::is_valid( const pixel& px ) const
  {
    boost::shared_array<double> g( px.get<2>() );

    if( g ) {

      const double* nodata_ptr( nodata_.get() );
      const double* g_ptr( g.get() );

      bool result( true );

      for( size_t k = 0; k < channels_; ++k, ++nodata_ptr, ++g_ptr ) {

        result = result && ( *g_ptr != *nodata_ptr );
      }

      return result;
    }

    return false;
  }

}


#ifndef CANVAS_IMAGE_HPP
#define CANVAS_IMAGE_HPP

#include <utility/mapped_memory.hpp>

#include <boost/tuple/tuple.hpp>

#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <CGAL/Bbox_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_2.h>

#include <gdal_priv.h>

#include <cstdlib>
#include <string>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

namespace canvas {

  class image : private boost::noncopyable {

  public:
    typedef boost::shared_ptr<image> ptr;

    typedef boost::shared_ptr<const image> const_ptr;

    typedef boost::tuple<
      double,                           // pixel size
      CGAL::Bbox_2,                     // bounding box
      std::string                       // projection reference tag
    > metadata;

    typedef boost::tuple<
      double,                           // x coordinate
      double,                           // y coordinate
      boost::shared_array<double>       // brightness values
    > pixel;

    image( const size_t& lines, const size_t& columns, const size_t& channels );

    image( const std::string& filename );

    virtual ~image();

    virtual void allocate() = 0;

    virtual void load() = 0;

    const size_t& get_lines() const;

    const size_t& get_columns() const;

    const size_t& get_channels() const;

    const double& get_nodata( size_t band_number ) const;

    boost::shared_ptr<metadata> get_metadata() const;

    void display_info( const std::string& tag = "" ) const;

    bool contains( const Kernel::Point_2& p ) const;

    pixel compute_position( const Kernel::Point_2& p ) const;

    virtual boost::shared_array<double> compute_values(
      const pixel& px ) const = 0;

    bool is_valid( const pixel& px ) const;

  protected:
    size_t lines_;

    size_t columns_;

    size_t channels_;

    boost::scoped_array<double> nodata_;

    boost::shared_ptr<metadata> metadata_;

    GDALDataset* dataset_;

  };

}

#endif

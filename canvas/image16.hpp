
#ifndef CANVAS_IMAGE16_HPP
#define CANVAS_IMAGE16_HPP

#include <canvas/image.hpp>

namespace canvas {

  class image16 : public image {

  public:
    typedef boost::shared_ptr<image16> ptr;

    typedef boost::shared_ptr<const image16> const_ptr;

    typedef utility::mapped_memory<boost::uint16_t> band;

    typedef boost::shared_ptr<band> band_ptr;

    image16( const size_t& lines,
             const size_t& columns,
             const size_t& channels = 1 );

    image16( const std::string& filename );

    virtual ~image16();

    void allocate( bool fill = false );

    void load();

    image16::ptr load( size_t l1, size_t c1, size_t l2, size_t c2 ) const;

    void write( const std::string& filename );

    boost::shared_array<double> compute_values( const pixel& px ) const;

    band_ptr get_band( size_t band_number ) const;

    image16::ptr compute_difference( const image16& other ) const;

  private:
    std::vector<band_ptr> bands_;

  };

}

#endif

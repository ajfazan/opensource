
#ifndef CANVAS_IMAGE8_HPP
#define CANVAS_IMAGE8_HPP

#include <canvas/image.hpp>

#include <utility/mapped_memory.hpp>

#include <vector>

namespace canvas {

  class image8 : public image {

  public:
    typedef boost::shared_ptr<image8> ptr;

    typedef boost::shared_ptr<const image8> const_ptr;

    typedef utility::mapped_memory<boost::uint8_t> band;

    typedef boost::shared_ptr<band> band_ptr;

    image8( const size_t& lines,
            const size_t& columns,
            const size_t& channels = 1 );

    image8( const std::string& filename );

    virtual ~image8();

    void allocate();

    void load();

    boost::shared_array<double> compute_values( const pixel& px ) const;

    band_ptr get_band( size_t band_number ) const;

  private:
    std::vector<band_ptr> bands_;

  };

}

#endif

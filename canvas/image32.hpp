
#ifndef CANVAS_IMAGE32_HPP
#define CANVAS_IMAGE32_HPP

#include <canvas/image.hpp>

namespace canvas {

  class image32 : public image {

  public:
    typedef boost::shared_ptr<image32> ptr;

    typedef boost::shared_ptr<const image32> const_ptr;

    typedef utility::mapped_memory<float> band;

    typedef boost::shared_ptr<band> band_ptr;

    typedef boost::tuple<
      std::vector<double>,  // minimum
      std::vector<double>,  // maximum
      std::vector<double>,  // mean
      std::vector<double>   // variance
    > stats;

    image32( const size_t& lines,
             const size_t& columns,
             const size_t& channels = 1 );

    image32( const std::string& filename );

    virtual ~image32();

    void allocate();

    void load();

    image32::ptr load( size_t l1, size_t c1, size_t l2, size_t c2 ) const;

    void write( const std::string& filename );

    boost::shared_array<double> compute_values( const pixel& px ) const;

    band_ptr get_band( size_t band_number ) const;

    image32::ptr compute_difference( const image32& other ) const;

    stats compute_stats() const;

  private:
    std::vector<band_ptr> bands_;

  };

}

#endif

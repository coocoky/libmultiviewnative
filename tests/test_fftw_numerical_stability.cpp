#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE FFTW_NUMERICAL_STABILITY
#include "boost/test/unit_test.hpp"
#include "test_fixtures.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include "multiviewnative.h"
#include "image_stack_utils.h"

#include "fftw3.h"

namespace mvn = multiviewnative;

typedef mvn::image_stack::array_view<3>::type subarray_view;

template<typename in_type, typename out_type = in_type>
  struct diff_squared {

    out_type operator()(const in_type& _first, const in_type& _second){

      out_type value = _first - _second;
      return (value*value);
    
    }
  
  };

BOOST_AUTO_TEST_SUITE(cpu_out_of_place)

BOOST_AUTO_TEST_CASE(ramp_of_primes_shape) {

  std::vector<size_t> signal_shape(3,0);
  signal_shape[mvn::row_major::z] = 19;
  signal_shape[mvn::row_major::y] = 17;
  signal_shape[mvn::row_major::x] = 13;
  mvn::image_stack stack(signal_shape);
  const size_t image_size_ = stack.num_elements();

  for(size_t i = 0;i<image_size_;++i)
    stack.data()[i] = i;

  std::vector<size_t> complex_shape(signal_shape);
  const size_t fft_out_of_place_size = std::accumulate(complex_shape.begin(), complex_shape.end(),1.,std::multiplies<size_t>());
  fftwf_complex* image_fourier = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * fft_out_of_place_size);

  float* d_src = 0;
  fftwf_complex* d_dst = 0;



  fftwf_plan image_fwd_plan = fftwf_plan_dft_r2c_3d(
      signal_shape[mvn::row_major::z], signal_shape[mvn::row_major::y], signal_shape[mvn::row_major::x], stack.data(), image_fourier, FFTW_MEASURE);
  fftwf_execute(image_fwd_plan);
  fftwf_destroy_plan(image_fwd_plan);

  float* image_result = (float*)fftwf_malloc(sizeof(float) * image_size_);
  fftwf_plan image_rev_plan = fftwf_plan_dft_c2r_3d(
      signal_shape[mvn::row_major::z], signal_shape[mvn::row_major::y], signal_shape[mvn::row_major::x], image_fourier, image_result, FFTW_ESTIMATE);
  fftwf_execute(image_rev_plan);

  float scale = 1.0 / (image_size_);
  for (unsigned index = 0; index < image_size_; ++index) {
    image_result[index] *= scale;
  }

  double l2norm = std::inner_product(image_result,
				     image_result + stack.num_elements(),
				     stack.data(),
				     0.,
				     std::plus<double>(),
				     diff_squared<float,double>()
				     );
  l2norm /= stack.num_elements();

  BOOST_MESSAGE(boost::unit_test::framework::current_test_case << "\tfftw-outofplace    shape(x,y,z)=" << signal_shape[mvn::row_major::x]<< ", " << signal_shape[mvn::row_major::y]<< ", " << signal_shape[mvn::row_major::z] << "\tl2norm = " << l2norm);
  BOOST_CHECK_LT(l2norm,1e-4);

  fftwf_destroy_plan(image_rev_plan);
  fftwf_free(image_result);
  fftwf_free(image_fourier);
}

BOOST_AUTO_TEST_SUITE_END()

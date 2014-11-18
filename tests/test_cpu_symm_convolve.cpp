#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE CPU_CONVOLUTION
#include "boost/test/unit_test.hpp"
#include "test_fixtures.hpp"
#include <numeric>
#include "multiviewnative.h"
#include "cpu_convolve.h"
#include "padd_utils.h"
#include "fft_utils.h"
#include "test_algorithms.hpp"
#include "image_stack_utils.h"

BOOST_FIXTURE_TEST_SUITE( convolution_works, multiviewnative::default_3D_fixture )

BOOST_AUTO_TEST_CASE( trivial_convolve )
{

  float* image = image_.data();
  multiviewnative::image_stack expected(image);
  
  float* kernel = new float[kernel_size_];
  std::fill(kernel, kernel+kernel_size_,0.f);

  inplace_cpu_convolution(image, &image_dims_[0], 
			  kernel,&kernel_dims_[0],
			  1);

  multiviewnative::image_stack_ref result(image, image_dims);
  
  float sum = std::accumulate(image, image + image_size_,0.f);
  BOOST_CHECK_CLOSE(multiviewnative::l2norm_within_limits(result, expected, .2,.8), 0.f, .00001);

  delete [] kernel;
}

BOOST_AUTO_TEST_CASE( identity_convolve )
{
  

  float sum_original = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  identity_kernel_.data(),&kernel_dims_[0],
  			  1);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  BOOST_CHECK_CLOSE(sum, sum_original, .00001);


}

BOOST_AUTO_TEST_CASE( horizontal_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_horizontal_.origin(), image_folded_by_horizontal_.origin() + image_folded_by_horizontal_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  horizont_kernel_.data(),&kernel_dims_[0],
  			  1);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);

  BOOST_CHECK_CLOSE(sum, sum_original, .00001f);

}

BOOST_AUTO_TEST_CASE( vertical_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_vertical_.origin(), image_folded_by_vertical_.origin() + image_folded_by_vertical_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  vertical_kernel_.data(),&kernel_dims_[0],
  			  1);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  BOOST_CHECK_CLOSE(sum, sum_original, .00001);


}

BOOST_AUTO_TEST_CASE( depth_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_depth_.origin(), image_folded_by_depth_.origin() + image_folded_by_depth_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  depth_kernel_.data(),&kernel_dims_[0],
  			  1);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  BOOST_CHECK_CLOSE(sum, sum_original, .00001);


}

BOOST_AUTO_TEST_CASE( all1_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_all1_.origin(), image_folded_by_all1_.origin() + image_folded_by_all1_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  all1_kernel_.data(),&kernel_dims_[0],
  			  1);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  BOOST_CHECK_CLOSE(sum, sum_original, .00001);


}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( parallel_convolution_works, multiviewnative::default_3D_fixture )

BOOST_AUTO_TEST_CASE( horizontal_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_horizontal_.origin(), image_folded_by_horizontal_.origin() + image_folded_by_horizontal_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  horizont_kernel_.data(),&kernel_dims_[0],
  			  boost::thread::hardware_concurrency()/2);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);

  BOOST_CHECK_CLOSE(sum, sum_original, .00001f);

}

BOOST_AUTO_TEST_CASE( vertical_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_vertical_.origin(), image_folded_by_vertical_.origin() + image_folded_by_vertical_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  vertical_kernel_.data(),&kernel_dims_[0],
  			  boost::thread::hardware_concurrency()/2);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  BOOST_CHECK_CLOSE(sum, sum_original, .00001);


}

BOOST_AUTO_TEST_CASE( depth_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_depth_.origin(), image_folded_by_depth_.origin() + image_folded_by_depth_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  depth_kernel_.data(),&kernel_dims_[0],
  			  boost::thread::hardware_concurrency()/2);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  BOOST_CHECK_CLOSE(sum, sum_original, .00001);


}

BOOST_AUTO_TEST_CASE( all1_convolve )
{
  

  float sum_original = std::accumulate(image_folded_by_all1_.origin(), image_folded_by_all1_.origin() + image_folded_by_all1_.num_elements(),0.f);
  inplace_cpu_convolution(image_.data(), &image_dims_[0], 
  			  all1_kernel_.data(),&kernel_dims_[0],
  			  boost::thread::hardware_concurrency()/2);

  float sum = std::accumulate(image_.origin(), image_.origin() + image_.num_elements(),0.f);
  BOOST_CHECK_CLOSE(sum, sum_original, .00001);


}
BOOST_AUTO_TEST_SUITE_END()

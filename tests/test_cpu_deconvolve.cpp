#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE CPU_DECONVOLVE
#include "boost/test/unit_test.hpp"
#include "tiff_fixtures.hpp"
#include "multiviewnative.h"
#include "convert_tiff_fixtures.hpp"

#include "cpu_convolve.h"
#include "padd_utils.h"
#include "fft_utils.h"
#include "cpu_kernels.h"
#include "test_algorithms.hpp"

using namespace multiviewnative;

static const ReferenceData reference;
static const first_2_iterations guesses;

BOOST_AUTO_TEST_SUITE( deconvolve_psi0_as_test_case  )

BOOST_AUTO_TEST_CASE( printf )
{

  typedef multiviewnative::zero_padd<multiviewnative::image_stack> wrap_around_padding;
  typedef multiviewnative::inplace_3d_transform<multiviewnative::image_stack> serial_transform;
  typedef multiviewnative::parallel_inplace_3d_transform<multiviewnative::image_stack> parallel_transform;
  typedef multiviewnative::cpu_convolve<wrap_around_padding,imageType, unsigned> default_convolution;

  //setup
  ReferenceData local_ref(reference);
  first_2_iterations local_guesses(guesses);
  workspace input;
  input.data_ = 0;
  fill_workspace(local_ref, input);

  image_stack input_psi = *local_guesses.psi(0);

  //convolve
  std::vector<unsigned> image_dim(3);
  std::vector<unsigned> kernel1_dim(3);
  std::vector<unsigned> kernel2_dim(3);
  view_data view_access;
  multiviewnative::image_stack integral = input_psi;
  std::stringstream filename;
  for(unsigned view = 0;view < input.num_views;++view){
    
    view_access = input.data_[view];
    
    std::copy(view_access.image_dims_    ,  view_access.image_dims_    +  3  ,  image_dim  .begin()  );
    std::copy(view_access.kernel1_dims_  ,  view_access.kernel1_dims_  +  3  ,  kernel1_dim.begin()  );
    std::copy(view_access.kernel2_dims_  ,  view_access.kernel2_dims_  +  3  ,  kernel2_dim.begin()  );
    
    std::cout << view << "/" << input.num_views << "\t"
	      << image_dim[0] << "x" << image_dim[1] << "x"<< image_dim[2] << " image convolved by "
      	      << kernel1_dim[0] << "x" << kernel1_dim[1] << "x"<< kernel1_dim[2] << " kernel1\n ";

    integral = input_psi; //copy psi to integral
    
    filename.str("");filename << "convolve_kernel1_" << view << "_input.tif";
    write_image_stack(integral,filename.str().c_str());
    
    //inplace convolve: psi x kernel1 -> psiBlurred :: (Psi*P_v)
    std::cout << "\nconvolve1: min/max integral " << multiviewnative::min_value(integral.data(), integral.num_elements()) << "/"
	      << multiviewnative::max_value(integral.data(), integral.num_elements()) 
	      << "\t min/max kernel1 " << multiviewnative::min_value(view_access.kernel1_, kernel1_dim[0]*kernel1_dim[1]*kernel1_dim[2]) << "/"
	      << multiviewnative::max_value(view_access.kernel1_, kernel1_dim[0]*kernel1_dim[1]*kernel1_dim[2]) << "\n\n";
      
    default_convolution convolver1(integral.data(), &image_dim[0], view_access.kernel1_ , &kernel1_dim[0]);
    convolver1.inplace<serial_transform>();

    filename.str("");filename << "convolve_kernel1_" << view << "_outcome.tif";
    write_image_stack(integral,filename.str().c_str());

    std::cout << "\ncomputeQuotient: min/max view " << multiviewnative::min_value(view_access.image_, integral.num_elements()) << "/"
	      << multiviewnative::max_value(view_access.image_, integral.num_elements()) 
	      << "\t min/max integral " << multiviewnative::min_value(integral.data(), integral.num_elements()) << "/"
	      << multiviewnative::max_value(integral.data(), integral.num_elements()) << "\n\n";


    filename.str("");filename << "computeQuotient_" << view << "_input.tif";
    write_image_stack(integral,filename.str().c_str());
    //view / psiBlurred -> psiBlurred :: (phi_v / (Psi*P_v))
    computeQuotient(view_access.image_,integral.data(),integral.num_elements());

    filename.str("");filename << "computeQuotient_" << view << "_outcome.tif";
    write_image_stack(integral,filename.str().c_str());

    std::cout << "\nconvolve2: min/max integral " << multiviewnative::min_value(integral.data(), integral.num_elements()) << "/"
	      << multiviewnative::max_value(integral.data(), integral.num_elements()) 
	      << "\t min/max kernel2 " << multiviewnative::min_value(view_access.kernel2_, kernel2_dim[0]*kernel2_dim[1]*kernel2_dim[2]) << "/"
	      << multiviewnative::max_value(view_access.kernel2_, kernel2_dim[0]*kernel2_dim[1]*kernel2_dim[2]) << "\n\n";

    filename.str("");filename<< "convolve_kernel2_" << view << "_input.tif";
    write_image_stack(integral,filename.str().c_str());
    //convolve: psiBlurred x kernel2 -> integral :: (phi_v / (Psi*P_v)) * P_v^{compound}
    default_convolution convolver2(integral.data(), &image_dim[0], view_access.kernel2_, &kernel2_dim[0]);
    convolver2.inplace<serial_transform>();
    filename.str("");filename<< "convolve_kernel2_" << view << "_outcome.tif";
    write_image_stack(integral,filename.str().c_str());

    std::cout << "\ncomputeFinalValues: "

	      << "min/max input_psi " << multiviewnative::min_value(input_psi.data(), input_psi.num_elements()) << "/"
	      << multiviewnative::max_value(input_psi.data(), input_psi.num_elements()) << "\t"

      	      << "min/max integral " << multiviewnative::min_value(integral.data(), integral.num_elements()) << "/"
	      << multiviewnative::max_value(integral.data(), integral.num_elements()) << "\t"

      	      << "min/max weights " << multiviewnative::min_value(view_access.weights_, integral.num_elements()) << "/"
	      << multiviewnative::max_value(view_access.weights_, integral.num_elements()) << "\n\n";



    filename.str("");filename<< "computeFinal_" << view << "_input.tif";
    write_image_stack(input_psi,filename.str().c_str());
    //computeFinalValues(input_psi,integral,weights)
    computeFinalValues(input_psi.data(), integral.data(), view_access.weights_, 
		       input_psi.num_elements(),
		       0, local_guesses.lambda_, local_guesses.minValue_);
    filename.str("");filename << "computeFinal_" << view << "_outcome.tif";
    write_image_stack(input_psi,filename.str().c_str());
    
    std::cout << "l2norm to psi1: " <<  multiviewnative::l2norm(input_psi.data(), local_guesses.psi(1)->data(), input_psi.num_elements()) << "\n";
  }

  //check norms
  float l2norm = multiviewnative::l2norm(input_psi.data(), local_guesses.psi(1)->data(), input_psi.num_elements());
  try{
    BOOST_REQUIRE_LT(l2norm, 1e-2);
  }
  catch (...){
    std::cout << "l2norm\t" << l2norm << "\nl1norm\t" << multiviewnative::l1norm(input_psi.data(), local_guesses.psi(1)->data(), input_psi.num_elements()) << "\n";
    write_image_stack(input_psi,"./reconstruct_manually_psi1.tif");
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( deconvolve_psi0  )
   
BOOST_AUTO_TEST_CASE( reconstruct_anything_1iteration )
{
  //setup
  ReferenceData local_ref(reference);
  first_2_iterations local_guesses(guesses);
  workspace input;
  input.data_ = 0;
  fill_workspace(local_ref, input);

  image_stack input_psi = *local_guesses.psi(0);

  //test
  inplace_cpu_deconvolve_iteration(input_psi.data(), input, 1, local_guesses.lambda_, local_guesses.minValue_);
  float sum_received = std::accumulate(input_psi.data(),input_psi.data() + input_psi.num_elements(),0.f);
  float sum_expected = std::accumulate(local_guesses.psi(1)->data(),local_guesses.psi(1)->data() + local_guesses.psi(1)->num_elements(),0.f);

  try{
    BOOST_REQUIRE_CLOSE(sum_expected, sum_received, 0.001);
  }
  catch (...){
    write_image_stack(input_psi,"./reconstruct_anything_psi1.tif");
  }

  //check norms
  float l2norm = multiviewnative::l2norm(input_psi.data(), local_guesses.psi(1)->data(), input_psi.num_elements());
  try{
    BOOST_REQUIRE_LT(l2norm, 1e-2);
  }
  catch (...){
    std::cout << "l2norm\t" << l2norm << "\nl1norm\t" << multiviewnative::l1norm(input_psi.data(), local_guesses.psi(1)->data(), input_psi.num_elements()) << "\n";
  }
  //tear-down
  delete [] input.data_;
}

BOOST_AUTO_TEST_CASE( reconstruct_anything_2iterations )
{
  //setup
  ReferenceData local_ref(reference);
  first_2_iterations local_guesses(guesses);
  workspace input;
  input.data_ = 0;
  fill_workspace(local_ref, input);
  image_stack input_psi = *local_guesses.psi(0);

  //test
  inplace_cpu_deconvolve_iteration(input_psi.data(), input, 1, local_guesses.lambda_, local_guesses.minValue_);
  inplace_cpu_deconvolve_iteration(input_psi.data(), input, 1, local_guesses.lambda_, local_guesses.minValue_);

  float sum_received = std::accumulate(input_psi.data(),input_psi.data() + input_psi.num_elements(),0.f);
  float sum_expected = std::accumulate(local_guesses.psi(2)->data(),local_guesses.psi(2)->data() + local_guesses.psi(2)->num_elements(),0.f);


  try{
    BOOST_REQUIRE_CLOSE(sum_expected, sum_received, 0.001);
  }
  catch (...){
    write_image_stack(input_psi,"./reconstruct_psi2.tif");
  }

  //check norms
  float l2norm = multiviewnative::l2norm(input_psi.data(), local_guesses.psi(2)->data(), input_psi.num_elements());
  BOOST_REQUIRE_LT(l2norm, 1e-2);

  //tear-down
  delete [] input.data_;
}

BOOST_AUTO_TEST_SUITE_END()
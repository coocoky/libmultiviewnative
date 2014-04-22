#define __MULTIVIEWNATIVE_CPP__

#include <vector>

#include "multiviewnative.h"
#include "cpu_convolve.h"
#include "padd_utils.h"
#include "fft_utils.h"

#include "cpu_kernels.h"

typedef multiviewnative::zero_padd<multiviewnative::image_stack> wrap_around_padding;
typedef multiviewnative::inplace_3d_transform<multiviewnative::image_stack> serial_transform;
typedef multiviewnative::parallel_inplace_3d_transform<multiviewnative::image_stack> parallel_transform;
typedef multiviewnative::cpu_convolve<wrap_around_padding,imageType, unsigned> default_convolution;

void inplace_cpu_convolution(imageType* im,
			     int* imDim,
			     imageType* kernel,
			     int* kernelDim,
			     int nthreads){

  
  
  unsigned image_dim[3];
  unsigned kernel_dim[3];

  std::copy(imDim, imDim + 3, &image_dim[0]);
  std::copy(kernelDim, kernelDim + 3, &kernel_dim[0]);
  default_convolution convolver(im, image_dim, kernel, kernel_dim);

  if(nthreads!=1){
    parallel_transform::set_n_threads(nthreads);
    convolver.inplace<parallel_transform>();
  }
  else{
    convolver.inplace<serial_transform>();
  }

}


void inplace_cpu_deconvolution(imageType* psi,int* psiDim, imageType* weights,
				 imageType* kernel1,int* kernel1Dim,
				 imageType* kernel2,int* kernel2Dim,
			       int nthreads, double lambda, imageType minValue){

  std::vector<unsigned> image_dim(3);
  std::vector<unsigned> kernel1_dim(3);
  std::vector<unsigned> kernel2_dim(3);

  std::copy(psiDim, psiDim + 3, &image_dim[0]);
  std::copy(kernel1Dim, kernel1Dim + 3, &kernel1_dim[0]);
  std::copy(kernel2Dim, kernel2Dim + 3, &kernel2_dim[0]);


  multiviewnative::image_stack_ref initial_psi(psi, image_dim);
  multiviewnative::image_stack current_psi = initial_psi;
  
  //convolve: initial_psi x kernel1 -> psiBlurred
  default_convolution convolver1(current_psi.data(), &image_dim[0], kernel1, &kernel1_dim[0]);
  convolver1.inplace<serial_transform>();

  //initial_psi / psiBlurred -> psiBlurred 
  computeQuotient(initial_psi.data(),current_psi.data(),current_psi.num_elements());

  //convolve: psiBlurred x kernel2 -> integral = current_psi
  default_convolution convolver2(current_psi.data(), &image_dim[0], kernel2, &kernel2_dim[0]);
  convolver2.inplace<serial_transform>();

  //computeFinalValues(initial_psi,integral,weights)
  computeFinalValues(initial_psi.data(), current_psi.data(), weights, 
		     current_psi.num_elements(),
		     0, lambda, minValue);

  //copy current image back to initial pointer
  initial_psi = current_psi;
}

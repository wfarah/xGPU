#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>

#include "xgpu_info.h"


#include "cube/cube.h"
#include "xgpu.h"

/*
  Data ordering for input vectors is (running from slowest to fastest)
  [time][channel][station][polarization][complexity]

  Output matrix has ordering
  [channel][station][station][polarization][polarization][complexity]
*/

int writeCorrelatorOutput(FILE file, Complex* cuda_matrix_h, size_t size)
{
  
  return 1;
}


int main(int argc, char** argv) {

  XGPUInfo xgpu_info;
  int xgpu_error = 0;

  unsigned int npol, nstation, nfrequency;
  // Get sizing info from library
  xgpuInfo(&xgpu_info);
  npol = xgpu_info.npol;
  nstation = xgpu_info.nstation;
  nfrequency = xgpu_info.nfrequency;

  //XXX: write a function to check meta data is correct w.r.t the sizing parameters
  //npol, nstation, nfrequency and ntime should match with xgpu_info. Error otherwise.

  //XXX all the following should be obtained from status buffer
  int device = 0; //XXX get device number (from status buffer, or env variable)
  FILE cc_file; //XXX define output filename. something like /mnt/buf[0-1]/corr/STEM/STEM.xgpu


  printf("Correlating %u stations with %u channels and integration length %u\n",
	 xgpu_info.nstation, xgpu_info.nfrequency, xgpu_info.ntime);

  XGPUContext context;
  ComplexInput* array_h_inp_tmp; //This is a BIG hack. Data are not ComplexInput, but with "USE4BIT", expansion happens on the GPU
  array_h_inp_tmp = malloc(1);
  context.array_h = array_h_inp_tmp; //input; this will stop xGPU from allocating input buffer
  context.matrix_h = NULL; //output; xGPU will allocate memory and take care of this internally

  xgpu_error = xgpuInit(&context, device);
  if(xgpu_error) {
    fprintf(stderr, "xgpuInit returned error code %d\n", xgpu_error); //XXX do something
  }

  Complex *cuda_matrix_h = context.matrix_h; 

  int observing = 1;
  void* hashpipe_ptr_tmp; //XXX get this from hashpipe
  while (observing)
  {
    context.array_h = (ComplexInput*) hashpipe_ptr_tmp; //XXX plug in hashpipe's data

    xgpu_error = xgpuCudaXengine(&context, SYNCOP_DUMP); //XXX figure out flags (85 Gbps with SYNCOP_DUMP)
    if(xgpu_error) 
      fprintf(stderr, "xgpuCudaXengine returned error code %d\n", xgpu_error);

    // Correlation done. Reorder the matrix and dump to disk
    xgpuReorderMatrix(cuda_matrix_h);
    // Can write another function here that integrates in time
    writeCorrelatorOutput(cc_file, cuda_matrix_h, context.matrix_len*sizeof(Complex)); //XXX 
  }

  free(array_h_inp_tmp);

  return xgpu_error;
}

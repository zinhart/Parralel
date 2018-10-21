#ifndef MULTI_CORE_ERROR_ERROR_HH
#define MULTI_CORE_ERROR_ERROR_HH
#include "macros.hh"
#if CUDA_ENABLED == true
#include <cublas_v2.h>
#endif
namespace zinhart
{
#if CUDA_ENABLED == true
  HOST const char * cublas_get_error_string(cublasStatus_t status);
  HOST std::int32_t check_cuda_api(cudaError_t result, const char * file, std::int32_t line);
  HOST std::int32_t check_cublas_api(cublasStatus_t result, const char * file, std::int32_t line);
#endif
}
#endif


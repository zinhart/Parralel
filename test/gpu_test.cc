#include "concurrent_routines/concurrent_routines.hh"
#include "concurrent_routines/concurrent_routines_error.hh"
#include "gtest/gtest.h"
#include <random>
#include <limits>
#include <iostream>
#include <list>
#if CUDA_ENABLED == true
#include <cublas_v2.h>
#endif
TEST(gpu_test, gemm_wrapper)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  //for any needed random uint
  std::uniform_int_distribution<std::uint8_t> uint_dist(1, std::numeric_limits<std::uint8_t>::max() );
  //for any needed random real
  std::uniform_real_distribution<float> real_dist(-5.5, 5.5);
  std::int32_t A_row = uint_dist(mt);
  std::int32_t A_col = uint_dist(mt);
  std::int32_t A_total_elements = A_row  * A_col;
  std::int32_t B_row = A_col;
  std::int32_t B_col = uint_dist(mt);
  std::int32_t B_total_elements = B_row * B_col;
  std::int32_t C_row = A_row;
  std::int32_t C_col = B_col;
  std::int32_t C_total_elements = C_row * C_col;
  std::cout<<"total pinned memory: "<<std::uint32_t(A_total_elements * B_total_elements * C_total_elements)<<" bytes.\n";
  std::cout<<" ";
  std::int32_t m, n, k, lda, ldb, ldc;
  double * A_host;
  double * B_host;
  double * C_host;
  double * A_host_copy;
  double * B_host_copy;
  double * C_host_copy;
  double * A_device, * B_device, * C_device;

  zinhart::check_cuda_api(cudaHostAlloc((void**)&A_host, A_total_elements * sizeof(double), cudaHostAllocDefault),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaHostAlloc((void**)&B_host, B_total_elements * sizeof(double), cudaHostAllocDefault),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaHostAlloc((void**)&C_host, C_total_elements * sizeof(double), cudaHostAllocDefault),__FILE__,__LINE__);

  zinhart::check_cuda_api(cudaHostAlloc((void**)&A_host_copy, A_total_elements * sizeof(double), cudaHostAllocDefault),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaHostAlloc((void**)&B_host_copy, B_total_elements * sizeof(double), cudaHostAllocDefault),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaHostAlloc((void**)&C_host_copy, C_total_elements * sizeof(double), cudaHostAllocDefault),__FILE__,__LINE__);

  for (std::int32_t i =0; i < A_row; ++i)
  {
	for(std::int32_t j = 0; j < A_col; ++j)
	{
	  A_host[zinhart::idx2r(i,j, A_col)] = real_dist(mt);
	}
  }

  for (std::int32_t i =0; i < B_row; ++i)
  {
	for(std::int32_t j = 0; j < B_col; ++j)
	{
	  B_host[zinhart::idx2r(i,j, B_col)] = real_dist(mt);
	}
  }

  for (std::int32_t i =0; i < C_row; ++i)
  {
	for(std::int32_t j = 0; j < C_col; ++j)
	{
	  C_host[zinhart::idx2r(i,j, C_col)] = 0.0f;
	}
  }

  for(std::int32_t i = 0; i < A_total_elements; ++i)
  {
	A_host_copy[i] = 0;
  }
  for(std::int32_t i = 0; i < B_total_elements; ++i)
  {
	B_host_copy[i] = 0;
  }
  for(std::int32_t i = 0; i < C_total_elements; ++i)
  {
	C_host_copy[i] = 0.0f;
  }

  zinhart::check_cuda_api(cudaMalloc((void**)&A_device,  A_total_elements * sizeof(double)),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaMalloc((void**)&B_device,  B_total_elements * sizeof(double)),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaMalloc((void**)&C_device,  C_total_elements * sizeof(double)),__FILE__,__LINE__);


  zinhart::check_cuda_api(cudaMemcpy(A_device, A_host, A_total_elements * sizeof(double), cudaMemcpyHostToDevice),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaMemcpy(B_device, B_host, B_total_elements * sizeof(double), cudaMemcpyHostToDevice),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaMemcpy(C_device, C_host, C_total_elements * sizeof(double), cudaMemcpyHostToDevice),__FILE__,__LINE__);

  cublasHandle_t context;
  zinhart::check_cublas_api(cublasCreate(&context),__FILE__,__LINE__);
  double alpha = 1;
  double beta = 1; 
  zinhart::gemm_wrapper(m,n,k,lda,ldb,ldc, A_row, A_col, B_row, B_col); 
  //sgemm here
  zinhart::check_cublas_api(cublasDgemm(context, CUBLAS_OP_N, CUBLAS_OP_N,
			  m, n, k,
			  &alpha,
			  B_device, lda,
			  A_device, ldb,
			  &beta,
			  C_device, ldc
			 ),__FILE__,__LINE__);/**/

  
  zinhart::check_cuda_api(cudaMemcpy(A_host_copy, A_device, A_total_elements * sizeof(double), cudaMemcpyDeviceToHost),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaMemcpy(B_host_copy, B_device, B_total_elements * sizeof(double), cudaMemcpyDeviceToHost),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaMemcpy(C_host_copy, C_device, C_total_elements * sizeof(double), cudaMemcpyDeviceToHost),__FILE__,__LINE__);
  zinhart::check_cublas_api(cublasDestroy(context),__FILE__,__LINE__);

  zinhart::serial_matrix_product(A_host, B_host, C_host, A_row, A_col, B_col);
/*  zinhart::print_matrix_row_major(A_host, A_row, A_col,"A_host");
  zinhart::print_matrix_row_major(A_host_copy, A_row, A_col, "A_host_copy");
  zinhart::print_matrix_row_major(B_host, B_row, B_col,"B_host");
  zinhart::print_matrix_row_major(B_host_copy, B_row, B_col, "B_host_copy");
  zinhart::print_matrix_row_major(C_host, C_row, C_col,"C_host");
  zinhart::print_matrix_row_major(C_host_copy, C_row, C_col, "C_host_copy");*/
  double epsilon = .0005;
  for(std::int32_t i = 0; i < A_total_elements; ++i)
  {
	ASSERT_NEAR(A_host[i], A_host_copy[i], epsilon);
  }
  for(std::int32_t i = 0; i < B_total_elements; ++i)
  {
	ASSERT_NEAR(B_host[i], B_host_copy[i], epsilon);
  }
  for(std::int32_t i = 0; i < C_total_elements; ++i)
  {
	ASSERT_NEAR(C_host[i],C_host_copy[i], epsilon);
  }

  zinhart::check_cuda_api(cudaFreeHost(A_host),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFreeHost(B_host),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFreeHost(C_host),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFreeHost(A_host_copy),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFreeHost(B_host_copy),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFreeHost(C_host_copy),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFree(A_device),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFree(B_device),__FILE__,__LINE__);
  zinhart::check_cuda_api(cudaFree(C_device),__FILE__,__LINE__); 
}

TEST(gpu_test, call_axps)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  //for any needed random uint
  std::uniform_int_distribution<std::uint32_t> uint_dist(1, std::numeric_limits<std::uint16_t>::max() );
  //for any needed random real
  std::uniform_real_distribution<float> real_dist(-5.5, 5.5);

  std::uint32_t N = uint_dist(mt);
  double a = real_dist(mt);
  double s = real_dist(mt);

  double * X_host{nullptr};
  double * X_host_copy{nullptr};
  double * X_device{nullptr};
  std::vector<zinhart::thread_pool::task_future<void>> serial_axps_results;

  X_host = new double[N];
  X_host_copy = new double[N];

  ASSERT_EQ(0, zinhart::check_cuda_api(cudaMalloc((void**)&X_device,  N * sizeof(double)),__FILE__,__LINE__));

  // initialize
  for(std::uint32_t i = 0; i < N; ++i)
  {
	X_host[i] = real_dist(mt);
	X_host_copy[i] = X_host[i];
  }

  // copy host to device
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaMemcpy(X_device, X_host, N * sizeof(double), cudaMemcpyHostToDevice),__FILE__,__LINE__));

  // call kernel 
  zinhart::call_axps(a, X_device, s,N); 
  // copy device to host
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaMemcpy(X_host, X_device, N * sizeof(double), cudaMemcpyDeviceToHost),__FILE__,__LINE__));
  // do serial axps
  for(std::uint32_t i = 0; i < N; ++i)
  {
	serial_axps_results.push_back(zinhart::default_thread_pool::push_task([](double a, double & x, double s){ x = a * x + s; }, a, std::ref(X_host_copy[i]), s));
  }

  // compare
  for(std::uint32_t i = 0; i < N; ++i)
  { 
    serial_axps_results[i].get();	
	ASSERT_EQ(X_host[i], X_host_copy[i]);
  }
  
  // deallocate
  delete X_host;
  delete X_host_copy;
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaFree(X_device),__FILE__,__LINE__));
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaDeviceReset(),__FILE__, __LINE__));
}



TEST(gpu_test, call_axps_async)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  //for any needed random uint
  std::uniform_int_distribution<std::uint32_t> uint_dist(1, std::numeric_limits<std::uint16_t>::max() );
  std::uniform_int_distribution<std::uint8_t> stream_dist(1, 10/*std::numeric_limits<std::uint8_t>::max()*/ );
  //for any needed random real
  std::uniform_real_distribution<float> real_dist(-5.5, 5.5);

  std::uint32_t N{uint_dist(mt)};
  std::uint32_t n_streams{stream_dist(mt)};
  double a {real_dist(mt)};
  double s {real_dist(mt)};

  double * X_host{nullptr};
  double * X_host_copy{nullptr};
  double * X_device{nullptr};
  cudaStream_t * stream{nullptr};
  std::list<zinhart::thread_pool::task_future<void>> serial_axps_results;

  ASSERT_EQ(0, zinhart::check_cuda_api(cudaHostAlloc((void**)&X_host, N * sizeof(double),cudaHostAllocDefault),__FILE__,__LINE__));
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaHostAlloc((void**)&X_host_copy, N * sizeof(double),cudaHostAllocDefault),__FILE__,__LINE__));
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaHostAlloc((void**)&stream, n_streams * sizeof(cudaStream_t),cudaHostAllocDefault),__FILE__,__LINE__));


  ASSERT_EQ(0, zinhart::check_cuda_api(cudaMalloc((void**)&X_device,  N * sizeof(double)),__FILE__,__LINE__));


  // initialize
  for(std::uint32_t i = 0; i < N; ++i)
  {
	X_host[i] = real_dist(mt);
	X_host_copy[i] = X_host[i];
  }

  for (std::uint32_t i = 0; i < n_streams; ++i)
  {
  	// copy host to device
	ASSERT_EQ(0, zinhart::check_cuda_api(cudaMemcpyAsync(X_device, X_host, N * sizeof(double), cudaMemcpyHostToDevice, stream[i]),__FILE__,__LINE__));
	// call kernel 
	ASSERT_EQ(0, zinhart::call_axps_async(a, X_device, s, N, stream[i])); 
	// copy device to host
	ASSERT_EQ(0, zinhart::check_cuda_api(cudaMemcpyAsync(X_host, X_device, N * sizeof(double), cudaMemcpyDeviceToHost, stream[i]),__FILE__,__LINE__));

	// do serial axps
	for(std::uint32_t j = 0; j < N; ++j)
	{
	  serial_axps_results.push_back(zinhart::default_thread_pool::push_task([](const double a, double & x, const double s){ x = a * x + s; }, a, std::ref(X_host_copy[j]), s));
	}
	
	cudaStreamSynchronize(stream[i]);
	double epsilon = 0.0005;
	// compare
	for(std::uint32_t j = 0; j < N; ++j)
	{ 	
  	  serial_axps_results.front().get();
	  ASSERT_NEAR(X_host[j], X_host_copy[j], epsilon);
	  serial_axps_results.pop_front();
	}

  }
  
  // deallocate
  cudaFreeHost(X_host);
  cudaFreeHost(X_host_copy);
  cudaFreeHost(stream);
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaFree(X_device),__FILE__,__LINE__));
  ASSERT_EQ(0, zinhart::check_cuda_api(cudaDeviceReset(),__FILE__, __LINE__));
}

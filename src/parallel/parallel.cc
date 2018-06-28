#include "concurrent_routines/parallel/parallel.hh"
namespace zinhart
{
  namespace parallel
  {
	namespace async
	{
	  /*
	   * CPU THREADED ROUTINES
	   * */
		HOST void saxpy(const std::uint32_t thread_id,
					 const std::uint32_t n_threads, const std::uint32_t n_elements, 
					 const float a, float * x, float * y 
			)
		{
		  std::uint32_t start = 0, stop = 0;
		  zinhart::serial::map(thread_id, n_threads, n_elements, start, stop);
		  //operate on y's elements from start to stop
		  for(std::uint32_t op = start; op < stop; ++op)
		  {
			y[op] = a * x[op] + y[op];
		  }
		}
	  /*
	   * CPU WRAPPERS IMPLEMENTATION
	   * */
		HOST void parallel_saxpy(
			  const float & a, float * x, float * y,
			  const std::uint32_t & n_elements, const thread_pool & default_thread_pool
			  )
		{ 
		  //to identify each thread
		  std::uint32_t thread_id = 0;
		  std::vector<std::thread> threads(default_thread_pool.size());
		  //initialize each thread
		  for(std::thread & t : threads)
		  {
			t = std::thread(saxpy, thread_id, default_thread_pool.size(), n_elements, a, x, y );
			++thread_id;
		  }
		  for(std::thread & t : threads)
			t.join();
		}
	}
  }
}

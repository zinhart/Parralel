#include "../concurrent_routines.hh"
#include <vector>
namespace zinhart
{

/*
 * CPU THREADED ROUTINES
 * */

  template<class InputIt, class OutputIt>
  void copy(InputIt input_it, OutputIt output_it,
		const std::uint32_t & thread_id, const std::uint32_t & n_elements, const std::uint32_t & n_threads)
  {
	//total number of operations that must be performed by each thread
  	const std::uint32_t n_ops = n_elements / n_threads; 
	//may not divide evenly
	const std::uint32_t remaining_ops = n_elements % n_threads;
	//if it's the first thread, start should be 0
	const std::uint32_t start = (thread_id == 0) ? n_ops * thread_id : n_ops * thread_id + remaining_ops;
	const std::uint32_t stop = n_ops * (thread_id + 1) + remaining_ops;
	//here stop start is how much we should increment the (output/input)_it
	for(std::uint32_t op = start; op < stop; ++op)
	{
	  *(output_it + op) = *(input_it + op);
	}
  }
  
/*
 * CPU WRAPPERS IMPLEMENTATION
 * */
  template<class InputIt, class OutputIt>
  OutputIt paralell_copy_cpu(InputIt first, InputIt last, OutputIt output_first, const std::uint32_t & n_threads)
  {
	//to identify each thread
	std::uint32_t thread_id = 0;
	const std::uint32_t n_elements = std::distance(first, last);
	std::vector<std::thread> threads(n_threads);
	//initialize each thread
	for(std::thread & t : threads)
	{
	  t = std::thread(copy<InputIt,OutputIt>, std::ref(first), std::ref(output_first), thread_id, n_elements, n_threads );
	  ++thread_id;
	}
	for(std::thread & t : threads)
	  t.join();
	return output_first;
  }
  

 /* template< class InputIt, class OutputIt >
  OutputIt paralell_accumalate_cpu( InputIt first, InputIt last, OutputIt d_first,
									const std::uint32_t & n_threads = MAX_CPU_THREADS	  )
  {
	//to identify each thread
	std::uint32_t thread_id = 0;
	std::vector<std::thread> threads(n_threads);
	//initialize each thread
	for(std::thread & t : threads)
	{
	  t = std::thread(saxpy, thread_id, n_threads, n_elements, a, x, y );
	  ++thread_id;
	}
	for(std::thread & t : threads)
	  t.join();
  }
  
  template < class InputIt, class UnaryFunction >
  UnaryFunction paralell_for_each_cpu(InputIt first, InputIt last, UnaryFunction f,
	  	                              const std::uint32_t & n_threads = MAX_CPU_THREADS  )
  
  {
  }
  template < class InputIt, class OutputIt, class UnaryOperation >
  OutputIt paralell_transform_each_cpu(InputIt first1, InputIt last1, OutputIt d_first, UnaryOperation unary_op,
									   const std::uint32_t & n_threads = MAX_CPU_THREADS )
  {
  }
  template < class ForwardIt, class Generator >
  void paralell_generate_cpu(ForwardIt first, ForwardIt last, Generator g,
	   const std::uint32_t & n_threads = MAX_CPU_THREADS)
  {
  }
  */
}

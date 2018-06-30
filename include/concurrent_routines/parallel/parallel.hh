#ifndef ZINHART_PARALLELL_HH
#define ZINHART_PARALLELL_HH
#include "../serial/serial.hh" // for map 
#include "vectorized/vectorized.hh"
#include "thread_pool.hh"
namespace zinhart
{
  namespace parallel
  {
	/*
	 * Asynchonous cpu methods. Since these are asynchrounous the called is expected to call resulsts[i].get() to ensure all threads have completed their work
	 * */
	namespace async
	{
	  template <class Precision_Type, class Container> 
		HOST void saxpy(const Precision_Type & a, Precision_Type * x, Precision_Type * y,const std::uint32_t & n_elements, Container & results,
		                         thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
	                            );
	  template<class InputIt, class OutputIt, class Container>
		HOST void copy(const InputIt & first, const InputIt & last, OutputIt & output_it, Container & results, 
			                        thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
								   );

	  template<class InputIt, class OutputIt, class UnaryPredicate, class Container>
		HOST void copy_if(const InputIt & first, const InputIt & last, OutputIt & output_it, UnaryPredicate pred, Container & results,
		                               thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
		                              );

	  template< class ForwardIt, class T, class Container >
		HOST void replace(ForwardIt & first, const ForwardIt & last, const T & old_value, const T & new_value, Container & results,
		                           thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
		                 	      );

	  template< class ForwardIt, class UnaryPredicate, class T, class Container >
		HOST void replace_if(ForwardIt & first, const ForwardIt & last, UnaryPredicate p, const T& new_value, Container & results,
		                              thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
									 );

	  template< class InputIt, class OutputIt, class T, class Container >
		HOST void replace_copy(InputIt & first, const InputIt & last, OutputIt & output_it, const T& old_value, const T& new_value, Container & results,
		                                    thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
										   );

	  template< class InputIt, class OutputIt, class UnaryPredicate, class T, class Container >
		HOST void replace_copy_if(const InputIt & first, const InputIt & last, OutputIt & output_it, UnaryPredicate p, const T& new_value, Container & results,
		                                       thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
											   );

	  template< class InputIt1, class InputIt2, class T, class Container>
		HOST void inner_product(const InputIt1 & first1, const InputIt1 & last1, const InputIt2 & first2, T & value, Container & results,
		                              thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
									 );
	  template<class InputIt1, class InputIt2, class T, class BinaryOperation1, class BinaryOperation2, class Container>
		HOST void inner_product(const InputIt1 & first1, const InputIt1 & last1, const InputIt2 & first2, T & value, BinaryOperation1 op1, BinaryOperation2 op2, Container & results,
		                              thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
									 );
	 
  	  template< class InputIt, class T, class Container >
  		HOST void accumulate(const InputIt & first, const InputIt & last, T & init, Container & results,
		                         thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
								);

	template < class InputIt, class UnaryFunction, class Container >
	  HOST void for_each(const InputIt & first, const InputIt & last, UnaryFunction f, Container & results,
		                                   thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
		                                  );
	
	template < class InputIt, class OutputIt, class UnaryOperation, class Container >
	  HOST void transform(const InputIt & first, const InputIt & last, OutputIt & output_first, UnaryOperation unary_op, Container & results,
		                               thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
		                              );

	template < class BidirectionalIt, class Generator, class Container >
	  HOST void generate(const BidirectionalIt & first, const BidirectionalIt & last, Generator g, Container & results,
		                          thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool()
								 );

	template <class Precision_Type, class Container>
	  HOST void kahan_sum(Precision_Type *& data, const std::uint32_t & data_size, Precision_Type & sum, Container & results,
		                  thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool());
	
	template <class Precision_Type, class Container>
	  HOST void neumaier_sum(Precision_Type *& data, const std::uint32_t & data_size, Precision_Type & sum, Container & results,
		                     thread_pool & default_thread_pool = zinhart::parallel::default_thread_pool::get_default_thread_pool());

	}// END NAMESPACE ASYNC
  }// END NAMESPACE PARALLEL
}// END NAMESPACE ZINHART
#include "ext/parallel.tcc"
#endif

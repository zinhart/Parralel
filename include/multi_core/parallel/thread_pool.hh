#ifndef THREAD_POOL_HH
#define THREAD_POOL_HH
#include <mutex>
#include <thread>
#include <future>
#include <functional>
#include <vector>
#include <multi_core/macros.hh>
#include <multi_core/parallel/thread_safe_queue.hh>
#include <multi_core/parallel/thread_safe_priority_queue.hh>
#include <functional>
#include <type_traits>
namespace zinhart
{
  namespace multi_core
  {
	namespace thread_pool
	{
		namespace tasks
		{
		  class thread_task_interface
		  {
			protected:
			  std::uint64_t priority;
			public:
			  HOST thread_task_interface() = default;
			  HOST ~thread_task_interface() = default;
			  thread_task_interface & operator =(thread_task_interface&&) = default;
			  HOST virtual void operator()() = 0;
			  HOST virtual bool operator < (const thread_task_interface & tti){return false;};
			  HOST virtual void set_priority(std::uint64_t priority)
			  {  }
			  HOST virtual std::uint64_t get_priority()const
			  { return 0; }
			  thread_task_interface(const thread_task_interface&) = delete;
			  thread_task_interface & operator =(const thread_task_interface&) = delete;

		  };

		  template <class Callable>
			class thread_task : public thread_task_interface
			{
			  private:
				  Callable callable;
			  public:
				  HOST thread_task(Callable && c)
				  {	this->callable = std::move(c); }
				  thread_task & operator =(thread_task&&) = default;
				  HOST virtual ~thread_task() = default;
				  HOST void operator()() override
				  { this->callable(); }
				  thread_task(const thread_task&) = delete;
				  thread_task & operator =(const thread_task&) = delete;
			};	

		  template <class Callable>
			class priority_thread_task : public thread_task_interface
			{
			  private:
				  Callable callable;
				  using thread_task_interface::priority;
			  public:
				  HOST priority_thread_task(std::uint64_t priority, Callable && c)
				  {	
					this->priority = priority;
					this->callable = std::move(c); 
				  }
				  priority_thread_task & operator =(priority_thread_task&&) = default;
				  virtual ~priority_thread_task() = default;
				  HOST void operator()() override
				  { this->callable(); }
				  HOST virtual bool operator < (const thread_task_interface & tti) override
				  { return get_priority() < tti.get_priority(); }
				  HOST virtual void set_priority(std::uint64_t priority) override
				  { this->priority = priority; }
				  HOST virtual std::uint64_t get_priority() const override
				  { return priority; }

				  priority_thread_task(const priority_thread_task&) = delete;
				  priority_thread_task & operator =(const priority_thread_task&) = delete;
			};


		  template <class T>
			class task_future
			{
			  private:
				  std::future<T> future;
			  public:
				  HOST task_future(std::future<T> && future)
				  {	this->future = std::move(future); }
				  HOST task_future(const task_future&) = delete;
				  HOST task_future & operator =(const task_future&) = delete;
				  task_future & operator =(task_future&&) = default;
				  task_future(task_future &&) = default;
				  HOST ~task_future()
				  {
					if (future.valid())
						future.get();
				  }
				  HOST bool valid()
				  { return future.valid(); }
				  HOST T get()
				  { return future.get(); }
			};
		}// END NAMESPACE TASKS
		

		enum class THREAD_POOL_STATE : bool {UP = true, DOWN = false};

		
	  template <class Thread_Safe_Queue>
		class thread_pool;

	  // an asynchonous thread pool
	  template <> 
		class thread_pool< thread_safe_queue< std::shared_ptr<tasks::thread_task_interface> > >
		{
		  public:
			HOST void down();
			// disable everthing
			HOST thread_pool(const thread_pool&) = delete;
			HOST thread_pool(thread_pool&&) = delete;
			HOST thread_pool & operator =(const thread_pool&) = delete;
			HOST thread_pool & operator =(thread_pool&&) = delete;
			HOST thread_pool(std::uint32_t n_threads = std::max(1U, MAX_CPU_THREADS - 1));
			HOST ~thread_pool(); 
			HOST std::uint32_t size() const;
			HOST void resize(std::uint32_t size);
			
			template<class Callable, class ... Args>
			  HOST auto add_task(Callable && c, Args&&...args) -> tasks::task_future<typename std::result_of<decltype(std::bind(std::forward<Callable>(c), std::forward<Args>(args)...))()>::type >
			  {
				auto bound_task = std::bind(std::forward<Callable>(c), std::forward<Args>(args)...); 
				using result_type = typename std::result_of<decltype(bound_task)()>::type;
				using packaged_task = std::packaged_task<result_type()>;
				using task_type = tasks::thread_task<packaged_task>;
				packaged_task task{std::move(bound_task)};
				tasks::task_future<result_type> result{task.get_future()};
				queue.push(std::make_shared<task_type>(std::move(task)));
				return result;
			  }
			
		    struct deffered_base
			{
			  virtual void operator ()() = 0;
			};
			// 1 idea
			template<class Callable, class ... Args>
			  struct deffered : public deffered_base
			  {
				std::function<void()> call_back;
				auto setup(Callable c, Args &&... args)->typename std::result_of<decltype(std::bind(std::forward<Callable>(c), std::forward<Args>(args)...))()>
				{
				  call_back = [c, args...]()
				  {
					(c)(args...);
				  };
				}
				void operator()() override
				{
				  call_back();
				}
			  };

			template<class Callable, class ... Args>
			  struct deffered2 : public deffered_base
			  {
				std::function<void()> call_back;
				Callable callable;
				std::tuple<Args...> params;
				/*
				deffered2(Callable && c,Args &&... args)
				{
				  callable = std::move(c);
				  params = std::make_tuple<Args...>(args...);
				}*/
				void setup2(Callable && c, Args &&... args)
				{ 
//				  callable = std::move(c);
//				  params = std::make_tuple<Args...>(args...);
				}
				auto get_function()-> Callable
				{
				  return callable;
				}
				auto get_params()->std::tuple<Args...>
				{
				  return params;
				}
				auto setup(Callable c, Args &&... args)->typename std::result_of<decltype(std::bind(std::forward<Callable>(c), std::forward<Args>(args)...))()>
				{
				  call_back = [c, args...]()
				  {
					(c)(args...);
				  };
				}
				void operator()() override
				{
				  call_back();
				}
			  };
			template<class Callable, class ... Args>
			  HOST void store_task(Callable && c, Args&&...args)
			  {
				// new strategy
				// since moving a packaged task affects the shared state of the right hand side
				// will save the Callable and args and invoke as a deferred function
				std::unique_ptr<deffered<Callable, Args...>> test;
				test->setup(std::forward<Callable>(c), std::forward<Args>(args)...);
				std::unique_ptr<deffered2<Callable, Args...>> test1;
				test1->setup2(std::forward<Callable>(c), std::forward<Args>(args)...);
			  }
		  private:
			THREAD_POOL_STATE thread_pool_state;
			std::vector<std::thread> threads;
			//std::queue<std::unique_ptr<deffered_base>> function_backlog;
			thread_safe_queue< std::shared_ptr<tasks::thread_task_interface> > queue;
			HOST void up(const std::uint32_t & n_threads);
			HOST void work();
		};

	  // an asynchonous thread pool with task scheduling
	  template <>
		class thread_pool<thread_safe_priority_queue<std::shared_ptr<tasks::thread_task_interface>>>
		{
		  public:

			// disable everthing
			HOST thread_pool(const thread_pool&) = delete;
			HOST thread_pool(thread_pool&&) = delete;
			HOST thread_pool & operator =(const thread_pool&) = delete;
			HOST thread_pool & operator =(thread_pool&&) = delete;
			HOST thread_pool(std::uint32_t n_threads = std::max(1U, MAX_CPU_THREADS - 1));
			HOST ~thread_pool(); 
			HOST std::uint32_t size() const;
			HOST void resize(std::uint32_t size);
			template<class Callable, class ... Args>
			  HOST auto add_task(std::uint64_t priority, Callable && c, Args&&...args) -> tasks::task_future< typename std::result_of<decltype(std::bind(std::forward<Callable>(c), std::forward<Args>(args)...))()>::type >
			  {
				auto bound_task     = std::bind(std::forward<Callable>(c), std::forward<Args>(args)...); 
				using result_type   = typename std::result_of<decltype(bound_task)()>::type;
				using packaged_task = std::packaged_task<result_type()>;
				using task_type     = tasks::priority_thread_task<packaged_task>;
				packaged_task task{std::move(bound_task)};
				tasks::task_future<result_type> result{task.get_future()};
				queue.push(std::make_shared<task_type>(priority, std::move(task)));
				return result;
			  }
		  private:
			THREAD_POOL_STATE thread_pool_state;
			std::vector<std::thread> threads;
			thread_safe_priority_queue< std::shared_ptr<tasks::thread_task_interface> > queue;
			HOST void up(const std::uint32_t & n_threads);
			HOST void down();
			HOST void work();
		};
	  using pool = thread_pool< thread_safe_queue< std::shared_ptr<tasks::thread_task_interface> > >;
	  using priority_pool = thread_pool< thread_safe_priority_queue< std::shared_ptr<tasks::thread_task_interface> > >;


	  pool & get_thread_pool();
	  void resize(std::uint32_t n_threads);
	  const std::uint32_t size();

	  template <class Callable, class ... Args>
		auto push_task(Callable && c, Args&&...args) -> tasks::task_future<typename std::result_of<decltype(std::bind(std::forward<Callable>(c), std::forward<Args>(args)...))()>::type >	
		{ return get_thread_pool().add_task(std::forward<Callable>(c), std::forward<Args>(args)...); }
	  
	  namespace priority_thread_pool
	  {
		priority_pool & get_priority_thread_pool();
		void resize(std::uint32_t n_threads);
		const std::uint32_t size();

		template <class Callable, class ... Args>
		  auto push_task(std::uint64_t priority, Callable && c, Args&&...args) -> tasks::task_future<typename std::result_of<decltype(std::bind(std::forward<Callable>(c), std::forward<Args>(args)...))()>::type >	
		  { return get_priority_thread_pool().add_task(priority, std::forward<Callable>(c), std::forward<Args>(args)...); }
	  }

	}// END NAMESPACE THREAD_POOL
  }// END NAMESPACE MULTI_CORE
}// END NAMESPACE ZINHART
#endif

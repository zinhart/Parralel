namespace zinhart
{
  namespace multi_core
  {
	template<class T, class Container>
	  HOST thread_safe_queue<T, Container>::thread_safe_queue()
  	  { wakeup(); }
	template<class T, class Container>
	  HOST void thread_safe_queue<T, Container>::wakeup()
	  { 
		std::lock_guard<std::mutex> local_lock(lock);
		queue_state = QUEUE_STATE::ACTIVE; 
		cv.notify_all();
	  }
	template<class T, class Container>
	  HOST void thread_safe_queue<T, Container>::shutdown()
	  {
		std::lock_guard<std::mutex> local_lock(lock);
		queue_state = QUEUE_STATE::INACTIVE;
		cv.notify_all();
	  }
	template<class T, class Container>
	  HOST thread_safe_queue<T, Container>::~thread_safe_queue()
	  { shutdown(); }
	template<class T, class Container>
	  HOST void thread_safe_queue<T, Container>::push(const T & item)
	  {
		std::lock_guard<std::mutex> local_lock(lock);
		// add item to the queue
		queue.push(item);
		// notify a thread that an item is ready to be removed from the queue
		cv.notify_one();
	  }
	template<class T, class Container>
	  HOST void thread_safe_queue<T, Container>::push(T && item)
	  {
		std::lock_guard<std::mutex> local_lock(lock);
		// add item to the queue
		queue.push(std::move(item));
		// notify a thread that an item is ready to be removed from the queue
		cv.notify_one();
	  }
	template<class T, class Container>
	  HOST bool thread_safe_queue<T, Container>::pop(T & item)
	  {
		std::lock_guard<std::mutex> local_lock(lock);
		if(queue.size() > 0)
		{
		  // avoid copying
		  item = std::move(queue.front());
		  // update queue
		  queue.pop();
		  // successfull write
		  return true;
		}
		// unsuccessfull write
		return false;
	  }
	template<class T, class Container>
	  HOST bool thread_safe_queue<T, Container>::pop_on_available(T & item)
	  {
		// Since the cv is locked upon -> std::unique_lock
		std::unique_lock<std::mutex> local_lock(lock);
		// basically block the current thread until an item is available, 
		// so calling this function before pushing items on to the queue is an error,
		// further wait conditions could be added here 
		cv.wait(local_lock, [this](){ return queue.size() > 0 || queue_state == QUEUE_STATE::INACTIVE; });
		
		// if an early termination signal is received then return an unsuccessfull write
		if (queue_state == QUEUE_STATE::INACTIVE)
			return false;
		// avoid copying
		item = std::move(queue.front());
		// update queue
		queue.pop();
		// successfull write
		return true;
	  }

	template<class T, class Container>
	  HOST std::uint32_t thread_safe_queue<T, Container>::size()
	  {
		std::lock_guard<std::mutex> local_lock(lock);
		return queue.size();
	  }
	template<class T, class Container>
	  HOST bool thread_safe_queue<T, Container>::empty()
	  {
		std::lock_guard<std::mutex> local_lock(lock);
		return queue.empty();
	  }
	template<class T, class Container>
	  HOST void thread_safe_queue<T, Container>::clear()
	  {
		std::lock_guard<std::mutex> local_lock(lock);
		while(queue.size() > 0)
		  queue.pop();
		cv.notify_all();
	  }
  }// END NAMESPACE MULTI_CORE
}// END NAMESPACE ZINHART

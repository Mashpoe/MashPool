#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <cassert>

class MashPool
{
public:

	MashPool(size_t threadCount = std::thread::hardware_concurrency());

	// joins all worker threads
	~MashPool();

	void addTask(std::function<void()> task);

	template<class F, class... Args>
	auto addTaskFuture(F&& f, Args&&... args)
		-> std::future<std::invoke_result_t<F, Args...>>;

	// waits until all tasks have been completed
	void wait();

private:

	std::vector<std::thread> workers;

	std::queue<std::function<void()>> tasks;
	std::atomic_uint taskCount;

	// synchronization
	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop;
};

inline MashPool::MashPool(size_t threadCount)
	: stop{ false }
{
	for (size_t i = 0; i < threadCount; ++i)
	{
		workers.emplace_back(
			[this]
			{
				for (;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock{ this->queueMutex };
						this->condition.wait(lock,
							[this] {
								return this->stop || !this->tasks.empty();
							}
						);

						if (this->stop && this->tasks.empty())
						{
							return;
						}

						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();

					// decrement taskCount only after completing a task
					--this->taskCount;
				}
			}
		);
	}
}

inline MashPool::~MashPool()
{
	{
		std::scoped_lock lock{ queueMutex };
		stop = true;
	}
	condition.notify_all();

	for (std::thread& worker : workers)
	{
		worker.join();
	}
}

inline void MashPool::addTask(std::function<void()> task)
{
	// increment "taskCount" before doing anything
	++taskCount;

	{
		std::scoped_lock lock{ queueMutex };

		// don't allow enqueueing after stopping the pool
		assert(!stop);

		tasks.emplace(task);
	}
	condition.notify_one();
}

template<class F, class... Args>
auto MashPool::addTaskFuture(F&& f, Args&&... args)
-> std::future<std::invoke_result_t<F, Args...>>
{
	// increment "taskCount" before doing anything
	++taskCount;

	using return_type = std::invoke_result_t<F, Args...>;

	// this has to be a shared pointer because an std::function cannot be created from a lambda that captures a non copy-constructible type.
	// the only way to fix this would be to create a custom std::function.
	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	{
		std::scoped_lock lock{ queueMutex };

		// don't allow enqueueing after stopping the pool
		assert(!stop);

		tasks.emplace([task = std::move(task)]() { (*task)(); });
	}
	condition.notify_one();

	return res;
}

inline void MashPool::wait()
{
	while (taskCount > 0);
}

MashPool
========

A simple C++ thread pool implementation, forked from <https://github.com/progschj/ThreadPool>.

Basic usage:
```cpp
// create thread pool with 4 worker threads
MashPool pool{ 4 };

// calculate sum of the first 100 positive integers, in parallel
std::atomic_int sum = 0;
for (int i = 1; i <= 100; ++i)
{
	pool.addTask([&sum, i]
		{
			sum += i;
		}
	);
}

// wait for all of the tasks to be executed
pool.wait();

// print sum (5050)
std::cout << "sum: " << sum << std::endl;
```
Or with futures:
```cpp
// create a pool with "std::thread::hardware_concurrency()" threads
MashPool pool;

// queue task and store future
auto result = pool.addTaskFuture([](int n) { return n * n; }, 10);

// print result from future (100)
std::cout << "future result: " << result.get() << std::endl;
```

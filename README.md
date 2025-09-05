MashPool
========

A simple C++ thread pool implementation, forked from <https://github.com/progschj/ThreadPool>.

This fork introduces new features along with some slight performance improvements. It has been updated to work in modern C++, but it unfortunately requires C++23 or later.

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
Passing additional function arguments:
```cpp
// function with some parameters
void task(std::atomic_int& sum, int n) { sum += n; }

// ...

MashPool pool{ 4 };

std::atomic_int sum = 0;
for (int i = 1; i <= 100; ++i)
{
	// be sure to use std::ref() or std::cref() when passing an argument by reference
	pool.addTask(task, std::ref(sum), i);
}

pool.wait();

// print sum (5050)
std::cout << "sum: " << sum << std::endl;
```
Usage with futures:
```cpp
// create a pool with "std::thread::hardware_concurrency()" threads
MashPool pool;

// queue task and store future
auto result = pool.addTaskFuture([](int n) { return n * n; }, 10);

// print result from future (100)
std::cout << "future result: " << result.get() << std::endl;
```

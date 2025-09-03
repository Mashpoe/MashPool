#include "MashPool.h"
#include <iostream>
#include <chrono>

uint64_t getTime()
{
	namespace sc = std::chrono;
	return sc::duration_cast<sc::milliseconds>(sc::system_clock::now().time_since_epoch()).count();
}

// these examples demonstrate two different ways MashPools might be used,
// while also measuring some of their overhead in a not-so-thorough benchmark.
int main()
{
	{
		std::cout << "running MashPool future..." << std::endl;

		uint64_t before = getTime();

		MashPool pool{ std::thread::hardware_concurrency() };
		std::vector<std::future<int>> results;

		for (int i = 0; i < 1000000; ++i)
		{
			results.emplace_back(
				pool.addTaskFuture([i]
					{
						return i * i;
					}
				)
			);
		}

		uint64_t sum = 0;
		for (auto&& result : results)
		{
			sum += result.get();
		}

		uint64_t after = getTime();

		std::cout << "MashPool future time: " << after - before << std::endl;
		std::cout << "MashPool future sum: " << sum << std::endl;
	}

	{
		std::cout << "running MashPool..." << std::endl;

		uint64_t before = getTime();

		MashPool pool{ std::thread::hardware_concurrency() };

		std::atomic_uint64_t sum = 0;

		for (int i = 0; i < 1000000; ++i)
		{
			pool.addTask([&sum, i]
				{
					sum += i * i;
				}
			);
		}

		uint64_t after = getTime();

		std::cout << "MashPool time: " << after - before << std::endl;
		std::cout << "MashPool sum: " << sum << std::endl;
	}

	{
		std::cout << "running control..." << std::endl;

		uint64_t before = getTime();

		uint64_t sum = 0;

		// this will just get optimized away in most cases,
		// but it's good for checking the sum.
		for (int i = 0; i < 1000000; ++i)
		{
			sum += i * i;
		}

		uint64_t after = getTime();

		std::cout << "control time: " << after - before << std::endl;
		std::cout << "control sum: " << sum << std::endl;
	}
}

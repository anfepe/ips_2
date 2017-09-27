#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>

#include <vector>

using namespace std::chrono;

void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n",
		maximum->get_reference(), maximum->get_index_reference());
}

void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n",
		minimum->get_reference(), minimum->get_index_reference());
}

void ParallelSort(int *begin, int *end)
{
	if (begin != end) 
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle); 
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

void CompareForAndCilk_For(long size)
{
	std::vector<int> normal_vec;
	cilk::reducer<cilk::op_vector<int>> red_vec; 

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (long i = 0; i < size; i++)
	{
		normal_vec.push_back(rand() % 20000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	cilk_for(long i = 0; i < size; i++)
	{
		red_vec->push_back(rand() % 20000 + 1);
	}
	high_resolution_clock::time_point t3 = high_resolution_clock::now();

	duration<double> normal_vec_time = (t2 - t1);
	duration<double> red_vec_time = (t3 - t2);

	printf("Array size: %d\n", size);
	printf("Normal vector time: %lf\n", normal_vec_time);
	printf("Reduction vector time: %lf\n\n", red_vec_time);
}


int main()
{
	srand((unsigned)time(0));

	__cilkrts_set_param("nworkers", "4");

	const long base_minmax_mass_size = 10000;
	const int minmax_mass_length = 4;
	const long minmax_mass_sizes[minmax_mass_length] = {
		base_minmax_mass_size, 
		10*base_minmax_mass_size, 
		50*base_minmax_mass_size, 
		100*base_minmax_mass_size
	};

	for (int i = 0; i < minmax_mass_length; i++)
	{
		const long mass_size = minmax_mass_sizes[i];
		int *mass_begin, *mass_end;
		int *mass = new int[mass_size]; 
		for(long j = 0; j < mass_size; ++j)
		{
			mass[j] = (rand() % 25000) + 1;
		}
		mass_begin = mass;
		mass_end = mass_begin + mass_size;

		printf("Array size: %d\n", mass_size);
		if (i == 0)
		{
			ReducerMaxTest(mass, mass_size);
			ReducerMinTest(mass, mass_size);
		}

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		ParallelSort(mass_begin, mass_end);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		duration<double> duration = (t2 - t1);
		printf("ParallelSort time: %lf\n", duration.count());

		if (i == 0)
		{
			ReducerMaxTest(mass, mass_size);
			ReducerMinTest(mass, mass_size);
		}
		printf("\n");
		delete [] mass;
	}

	const int fill_mass_length = 8;
	const long fill_mass_sizes[fill_mass_length] = {
		1000000,
		100000,
		10000,
		1000,
		500,
		100,
		50,
		10
	};

	for (int i = 0; i < fill_mass_length; i++)
	{
		CompareForAndCilk_For(fill_mass_sizes[i]);
	}

	return 0;
}
#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include "rdtscp_timer/timing.hh"

#define MB 1048576

using namespace std;

// Time the allocations of various sizes
static void testAllocation(double clock_speed_ghz) {
	vector<double> times;
	vector<double> sizes_bytes;
	vector<char*> ptrs;
	Timer t(clock_speed_ghz);

	const int N_ALLOCS = 1;
	const int N_TRIALS = 100000;
	

	// file with random malloc sizes
	int malloc_size[N_TRIALS];
	FILE *fp;
	fp = fopen("list_of_random_number.txt", "r");

	if(fp == NULL)
	{
		perror("Error opening file\n");
	}

	for(int i = 0; i < N_TRIALS; i++)
	{
		char temp[10];
		if(fgets(temp, 10, fp))
			malloc_size[i] = atoi(temp);
	} 

	for (int s = 0; s < N_ALLOCS; s++) {
		for (int i = 0; i < N_TRIALS; i++) {
			void *vptr = NULL;
			char *a = NULL;
			t.tick();
			//assert(!posix_memalign(static_cast<void**>(&vptr), 2*MB, 2*MB));
			//memset(vptr, rand(), 2*MB); // make sure we actually have pages allocated
			assert(!posix_memalign(static_cast<void**>(&vptr), 16, malloc_size[i]));
			memset(vptr, rand(), malloc_size[i]); // make sure we actually have pages allocated
			t.tock();
	
			#if defined(__GNUC__)
    	//	__builtin___clear_cache((char *) vptr, (char *) vptr + 2*MB);
    		__builtin___clear_cache((char *) vptr, (char *) vptr + malloc_size[i]);
			#endif

			assert(!(reinterpret_cast<unsigned long long>(a) & 0xFFFFF));
			times.push_back(t.get_time());
			ptrs.push_back(a);
		}

		for (int i = 0; i < N_TRIALS - 1; i++)
			cout << times[i] << ", ";
		cout << times[N_TRIALS - 1] << endl;

		//for (int i = 0; i < ptrs.size(); i++)
			//delete [] ptrs[i];

		ptrs.clear();
		times.clear();
	}
	fclose(fp);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		cerr << "WARNING: No system clock speed provided, RDTSCP timing will not be used!" << endl;
		testAllocation(-1);
	} else {
		testAllocation(atof(argv[1]));
	}

	return 0;
}

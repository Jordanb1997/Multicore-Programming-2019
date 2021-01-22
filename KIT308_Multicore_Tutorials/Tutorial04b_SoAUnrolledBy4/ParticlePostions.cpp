#include <stdio.h>
#include <time.h>
#include <immintrin.h> // AVX through SSE through MMX

// number of particles
#define N 1024*1024

// number of times to time each test
#define TIMES 1024

struct Position
{
	float x;
	float y;
	float z;
};

// particle positions stored as an structure of arrays (SoA)
struct Positions
{
	__declspec(align(64)) float x[N];
	__declspec(align(64)) float y[N];
	__declspec(align(64)) float z[N];
};
Positions particle_positions;

int main(void)
{
	// timing variables
	clock_t start, finish;

	// amount to move each particle by
	Position offset = { 10, 5, 7 };

	// size of particle positions list
	printf("sizeof(particle_positions_SoA) = %zd\n", sizeof(particle_positions));

	start = clock();

	// loop TIMES times to ensure proper timing
	for (int times = 0; times < TIMES; times++)
	{
		
		__m128 x = _mm_set_ps1(offset.x);
		__m128 y = _mm_set_ps1(offset.y);
		__m128 z = _mm_set_ps1(offset.z);

		// loop through whole particle position list and move each one
		for (int i = 0; i < N; i += 4)
		{
			//TODO: use _mm_load_ps to load four x values from particle_positions x array into a __m128 variable
			//TODO: use _mm_add_ps to add above value add the x offset value (and store it somewhere)
			//TODO: use _mm_store_ps to store the result back into particle_positions x array

			particle_positions.x[i] += offset.x;
			particle_positions.x[i + 1] += offset.x;
			particle_positions.x[i + 2] += offset.x;
			particle_positions.x[i + 3] += offset.x;

			//TODO: use _mm_load_ps to load four y values from particle_positions y array into a __m128 variable
			//TODO: use _mm_add_ps to add above value add the y offset value (and store it somewhere)
			//TODO: use _mm_store_ps to store the result back into particle_positions y array
			particle_positions.y[i] += offset.y;
			particle_positions.y[i + 1] += offset.y;
			particle_positions.y[i + 2] += offset.y;
			particle_positions.y[i + 3] += offset.y;

			//TODO: use _mm_load_ps to load four z values from particle_positions z array into a __m128 variable
			//TODO: use _mm_add_ps to add above value add the z offset value (and store it somewhere)
			//TODO: use _mm_store_ps to store the result back into particle_positions z array
			particle_positions.z[i] += offset.z;
			particle_positions.z[i + 1] += offset.z;
			particle_positions.z[i + 2] += offset.z;
			particle_positions.z[i + 3] += offset.z;
		}
	}

	finish = clock();

	// output time taken for this run
	printf("time taken %ums\n", finish - start);

	// output a selection of the particle positions as rough check
	for (int i = 0; i < N; i += N / 7)
	{
		printf("%14d @ (%f, %f, %f)\n", i, particle_positions.x[i],
			particle_positions.y[i], particle_positions.z[i]);
	}

	return 0;
}

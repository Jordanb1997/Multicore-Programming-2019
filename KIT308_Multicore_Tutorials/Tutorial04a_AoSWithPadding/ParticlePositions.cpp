#include <stdio.h>
#include <time.h>

// number of particles
#define N 1024*1024

// number of times to time each test
#define TIMES 1024

// particle positions stored as an array of structures (AoS)
struct Position
{
	float x;
	//char padding[4]; // artificially force seperation of values
	float y;
	//char padding2[4]; // artificially force seperation of values
	float z;
	//char padding3[4]; // artificially force seperation of values
};

struct Positions
{
	float x[N];
	float y[N];
	float z[N];
};

Positions particle_positions;

//Position particle_positions[N];

// move a particle position by a specified offset
void Offset(Position& p, const Position& offset) {
	p.x += offset.x;
	p.y += offset.y;
	p.z += offset.z;
}

int main(void)
{
	// timing variables
	clock_t start, finish;

	// amount to move each particle by
	Position offset = { 10, 5, 7 };

	// size of particle positions list
	printf("sizeof(particle_positions) = %zd\n", sizeof(particle_positions));

	start = clock();

	// loop TIMES times to ensure proper timing
	for (int times = 0; times < TIMES; times++)
	{
		// loop through whole particle position list and move each one
		for (int i = 0; i < N; i += 4)
		{
			particle_positions.x[i] += offset.x;
			particle_positions.x[i + 1] += offset.x;
			particle_positions.x[i + 2] += offset.x;
			particle_positions.x[i + 3] += offset.x;
			particle_positions.y[i] += offset.y;
			particle_positions.y[i + 1] += offset.y;
			particle_positions.y[i + 2] += offset.y;
			particle_positions.y[i + 3] += offset.y;
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

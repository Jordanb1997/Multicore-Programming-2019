// Tutorial01_Supersampling.cpp 
// Basic (single threaded) (CPU) Mandelbrot generation with supersampling

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bmp.h"
#include "Mandelbrot.h"

// buffer for colour information
unsigned int buffer[MAX_WIDTH * MAX_HEIGHT];


static void generate(unsigned int samples, unsigned int iterations, float centrex, float centrey, float scaley,
	unsigned int width, unsigned int height, unsigned int* out) 
{
	// output (hopefully helpful) generation message
	printf("generating at (%f, %f) with scale %f and %d iterations at size %dx%d per pixel at AA at %d\n", 
		centrex, centrey, scaley, iterations, width, height, samples);

	// timing variables
	clock_t start, finish;

	// calculate mandelbrot and time how long it takes
	start = clock();
	
	mandelbrot(samples, iterations, centrex, centrey, scaley, width, height, out);
	finish = clock();

	printf("time taken: %ums\n", finish - start);
}

int main(int argc, char **argv)
{
	// size of output
	unsigned int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;

	//number of samples
	unsigned int sample = DEFAULT_SAMPLE_PER_PIXEL;

	// "accuracy" of calculation
	unsigned int iterations = DEFAULT_MAX_ITER;

	// area of interest (centre coordinates on the complex plane and height of area)
	float centrex = DEFAULT_CENTRE_X;
	float centrey = DEFAULT_CENTRE_Y;
	float scaley = DEFAULT_SCALE_Y;

	//TODO: add a variable for the current number of samples to be done per pixel; prob not right
	unsigned int sample_to_do = sample;
	// (e.g. it's value being 1 means 1 sample per pixel, 2 means 2x2 samples per pixel, etc.)

	// read any command-line arguments
	for (int i = 1; i < argc; i++) 
	{
		if (strcmp(argv[i], "-iterations") == 0)
		{
			iterations = atoi(argv[++i]);
		} 
		else if (strcmp(argv[i], "-size") == 0)
		{
			width = atoi(argv[++i]);
			height = atoi(argv[++i]);
		} 
		else if (strcmp(argv[i], "-centre") == 0)
		{
			centrex = (float) atof(argv[++i]);
			centrey = (float) atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-scale") == 0)
		{
			scaley = (float) atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-samples") == 0)
		{
			sample = atoi(argv[++i]);
		}
		else
		{
			fprintf(stderr, "unknown argument: %s\n", argv[i]);
		}
	}

	// generate Mandelbrot fractal in global buffer
	generate(sample, iterations, centrex, centrey, scaley, width, height, buffer);

	// output (uncompressed) bmp file
	write_bmp("output.bmp", width, height, (char*) buffer);

	// success!
	return 0;
}


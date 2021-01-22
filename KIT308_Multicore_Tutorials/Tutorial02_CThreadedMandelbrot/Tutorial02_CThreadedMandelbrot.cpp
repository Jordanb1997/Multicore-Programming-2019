// Tutorial02_CThreadedMandelbrot.cpp 
// Basic multithreaded (CPU) Mandelbrot generation with supersampling

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bmp.h"
#include "Mandelbrot.h"

// buffer for colour information
unsigned int buffer[MAX_WIDTH * MAX_HEIGHT];

struct ThreadData {
	unsigned int threadid;
	unsigned int iterations;
	float centrex;
	float centrey;
	float scaley;
	unsigned int samples;
	unsigned int width;
	unsigned int height;
	unsigned int* out;

} threadData;

DWORD __stdcall startThreads(LPVOID td)
{
	ThreadData* tdata = (ThreadData*) td;

	mandelbrot(tdata->threadid, tdata->iterations, tdata->centrex, tdata->centrey, tdata->scaley, tdata->samples, tdata->width, tdata->height, tdata->out);

	ExitThread(NULL);
}
static void generate(unsigned int threads, unsigned int iterations, float centrex, float centrey, float scaley,
	unsigned int samples, unsigned int width, unsigned int height, unsigned int* out)
{
	// output (hopefully helpful) generation message
	printf("generating with %d threads at (%f, %f) with scale %f and %d iterations at size %dx%d with %d samples per pixel\n", 
		threads, centrex, centrey, scaley, iterations, width, height, samples * samples);

	// timing variables
	clock_t start, finish;

	// calculate mandelbrot and time how long it takes
	start = clock();
 
	HANDLE* theThreads = new HANDLE[threads];
 
	ThreadData* threadDatas = new ThreadData[threads];

	float scaleperthread = scaley / threads;
	for (unsigned int t = 0; t < threads; t++)
	{
		threadDatas[t].threadid = t;
		threadDatas[t].iterations = iterations;
		threadDatas[t].centrex = centrex;
		threadDatas[t].centrey = centrey - scaley /2  + scaleperthread /2 + t * scaleperthread;
		threadDatas[t].scaley = scaley/threads;
		threadDatas[t].samples = samples;
		threadDatas[t].width = width;
		threadDatas[t].height = height/threads;
		threadDatas[t].out = out + width * height / threads * t;

		theThreads[t] = CreateThread(NULL, 0, startThreads, (LPVOID)(ThreadData *)&threadDatas[t], 0, NULL);
		if (theThreads[t] == NULL)
		{
			printf("ERROR. Return code from CreateThread() is %d\n", GetLastError());
			exit(-1);
		}

	}
	
	DWORD wait = WaitForMultipleObjects(threads, theThreads, TRUE, INFINITE);
	switch (wait)
	{
		// ghEvents[0] was signaled
		case WAIT_OBJECT_0 + 0:
			printf("First event was signaled.\n");
			break;

		// ghEvents[1] was signaled
		case WAIT_OBJECT_0 + 1:
			printf("Second event was signaled.\n");
			break;

		case WAIT_TIMEOUT:
			printf("Wait timed out.\n");
			break;
			// Return value is invalid.
		default:
			printf("Wait error: %d\n", GetLastError());
			ExitProcess(0);
	}
	//TODO: delete the dynamically declared arrays
	//for (unsigned int a = 0; a < threads; a++)
	//{
	//	for (unsigned int b = 0; b < threads; b++)
	//	{
	//		free(&threadDatas[b]);
	//	}
	//	//free(threadDatas);
	//	for (unsigned int c = 0; c < threads; c++)
	//	{
	//		free(theThreads[c]);
	//	}
	//	//free(theThreads);
	//}

	finish = clock();

	printf("time taken: %ums\n", finish - start);
}

int main(int argc, char **argv)
{
	// number of threads to run on
	unsigned int threads = DEFAULT_THREADS;

	// size of output
	unsigned int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;

	// "accuracy" of calculation
	unsigned int iterations = DEFAULT_MAX_ITER;

	// area of interest (centre coordinates on the complex plane and height of area)
	float centrex = DEFAULT_CENTRE_X;
	float centrey = DEFAULT_CENTRE_Y;
	float scaley = DEFAULT_SCALE_Y;

	unsigned int samples = DEFAULT_SAMPLES;

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
			samples = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-threads") == 0)
		{
			threads = atoi(argv[++i]);
		}
		else
		{
			fprintf(stderr, "unknown argument: %s\n", argv[i]);
		}
	}

	// generate Mandelbrot fractal in global buffer
	generate(threads, iterations, centrex, centrey, scaley, samples, width, height, buffer);

	// output (uncompressed) bmp file
	write_bmp("output.bmp", width, height, (char*) buffer);

	// success!
	return 0;
}


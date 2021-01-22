// Tutorial07_OpenCLMandelbrot.cpp 
// OpenCL Mandelbrot generation with supersampling

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#pragma warning(disable: 4996) // so clCreateCommandQueue still works
#include <CL/cl.h>

#include "LoadCL.h"
#include "bmp.h"
#include "Mandelbrot.h"

// buffer for colour information
unsigned int buffer[MAX_WIDTH * MAX_HEIGHT];

static void generate(unsigned int iterations, float centrex, float centrey, float scaley,
	unsigned int samples, unsigned int width, unsigned int height, unsigned int* out)
{
	// output (hopefully helpful) generation message
	printf("generating at (%f, %f) with scale %f and %d iterations at size %dx%d with %d samples per pixel\n",
		centrex, centrey, scaley, iterations, width, height, samples * samples);

	// timing variables
	clock_t start, finish;

	// calculate mandelbrot and time how long it takes
	start = clock();

	cl_int err;
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem clBufferOut;

	// start at 0,0
	size_t workOffset[] = { 0, 0 };

	// rendering a width x height image
	size_t workSize[] = { width, height };

	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS)
	{
		printf("\nError calling clGetPlatformIDs. Error code: %d\n", err);
		exit(1);
	}

	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err != CL_SUCCESS) {
		printf("Couldn't find any devices\n");
		exit(1);
	}

	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a context\n");
		exit(1);
	}

	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create the command queue\n");
		exit(1);
	}

	program = clLoadSource(context, "Mandelbrot.cl", &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't load/create the program\n");
		exit(1);
	}

	err = clBuildProgram(program, 0, NULL, NULL /*"-save-temps=C:/TEMP/OCL"*/, NULL, NULL);
	if (err != CL_SUCCESS) {
		char *program_log;
		size_t log_size;

		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		program_log = (char*)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}

	kernel = clCreateKernel(program, "mandelbrot", &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create the kernel\n");
		exit(1);
	}
	clBufferOut = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(buffer), &buffer, &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferOut object\n");
		exit(1);
	}

	err = clSetKernelArg(kernel, 0, sizeof(unsigned int), &iterations);
	err = clSetKernelArg(kernel, 1, sizeof(float), &centrex);
	err = clSetKernelArg(kernel, 2, sizeof(float), &centrey);
	err = clSetKernelArg(kernel, 3, sizeof(float), &scaley);
	err = clSetKernelArg(kernel, 4, sizeof(unsigned int), &samples);
	err = clSetKernelArg(kernel, 5, sizeof(cl_mem), &clBufferOut);
	if (err != CL_SUCCESS) {
		printf("Couldn't set the kernel 0 argument\n");
		exit(1);
	}

	err = clEnqueueNDRangeKernel(queue, kernel, 2, workOffset, workSize, NULL, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Couldn't enqueue the kernel execution command\n");
		exit(1);
	}

	err = clEnqueueReadBuffer(queue, clBufferOut, CL_TRUE,0, sizeof(buffer), buffer,0,NULL,NULL );
	//: read the results with clEnqueueReadBuffer
	//: error check the read
	if (err != CL_SUCCESS) {
		printf("Couldn't enqueue the read buffer command\n");
		exit(1);
	}

	// clean up
	clReleaseMemObject(clBufferOut);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseContext(context);

	finish = clock();

	printf("time taken: %ums\n", finish - start);
}

int main(int argc, char **argv)
{
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
			centrex = (float)atof(argv[++i]);
			centrey = (float)atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-scale") == 0)
		{
			scaley = (float)atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-samples") == 0)
		{
			samples = atoi(argv[++i]);
		}
		else
		{
			fprintf(stderr, "unknown argument: %s\n", argv[i]);
		}
	}

	// generate Mandelbrot fractal in global buffer
	generate(iterations, centrex, centrey, scaley, samples, width, height, buffer);

	// output (uncompressed) bmp file
	write_bmp("output.bmp", width, height, (char*)buffer);

	// success!
	return 0;
}

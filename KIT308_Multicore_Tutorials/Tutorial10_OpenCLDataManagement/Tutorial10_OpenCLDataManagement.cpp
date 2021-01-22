#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "LoadCL.h"

struct Pair
{
	unsigned short littleNum;
	int bigNum;
};

struct InnerData
{
	char c;
	int x;
	Pair* pairs;
	char d;
};

struct ComplexData
{
	int* values;
	InnerData* inner;
};

int main(int argc, char **argv)
{
	// set a random seed because
	srand(17372);

	ComplexData data;

	// intialise the data structure with some random stuff and output some elements
	data.values = new int[100];
	data.inner = new InnerData[4];

	for (int i = 0; i < 100; i++) data.values[i] = rand();
	for (int i = 0; i < 4; i++)
	{
		data.inner[i].c = rand() % 26 + 65;
		data.inner[i].x = rand();
		data.inner[i].d = rand() % 26 + 96;
		data.inner[i].pairs = new Pair[50];

		for (int j = 0; j < 50; j++)
		{
			data.inner[i].pairs[j].littleNum = rand() % 65536;
			data.inner[i].pairs[j].bigNum = rand();
		}
	}

	printf("---------------\nCPU-side data\n");
	int sum = 0;
	for (int i = 0; i < 100; i++) sum += data.values[i];
	printf("%d, %d, ... %d, %d :: %d\n", data.values[0], data.values[1], data.values[98], data.values[99], sum);
	for (int i = 0; i < 4; i++)
	{
		printf("\ninner %d: %c, %d, %c\n", i, data.inner[i].c, data.inner[i].x, data.inner[i].d);

		sum = 0;
		for (int j = 0; j < 50; j++)
		{
			sum += data.inner[i].pairs[j].littleNum;
		}
		printf("%d, %d, ... %d, %d :: %d\n", data.inner[i].pairs[0].littleNum, data.inner[i].pairs[1].littleNum, 
			data.inner[i].pairs[48].littleNum, data.inner[i].pairs[49].littleNum, sum);

		sum = 0;
		for (int j = 0; j < 50; j++)
		{
			sum += data.inner[i].pairs[j].bigNum;
		}
		printf("%d, %d, ... %d, %d :: %d\n", data.inner[i].pairs[0].bigNum, data.inner[i].pairs[1].bigNum,
			data.inner[i].pairs[48].bigNum, data.inner[i].pairs[49].bigNum, sum);

	}

	cl_int err;
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem clBuffer1;
	cl_mem clBuffer2;
	cl_mem clBuffer3;
	cl_mem clBuffer4;
	cl_mem clBuffer5;
	cl_mem clBuffer6;
	cl_mem clBuffer7;
	size_t workOffset[] = { 0 };
	size_t workSize[] = { 1 };

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

	program = clLoadSource(context, "OutputData.cl", &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't load/create the program\n");
		exit(1);
	}

	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
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

	kernel = clCreateKernel(program, "func", &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create the kernel\n");
		exit(1);
	}

	//: will need multiple buffers
	clBuffer1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(ComplexData), &data, &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}

	clBuffer2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &data.values, &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}

	clBuffer3 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(InnerData), &data.inner, &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}

	clBuffer4 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Pair), &data.values[0], &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}

	clBuffer5 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Pair), &data.values[1], &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}

	clBuffer6 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Pair), &data.values[2], &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}

	clBuffer7 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Pair), &data.values[3], &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}
	//: will need to pass all the buffers through
	
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuffer1);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &clBuffer2);
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &clBuffer3);
	err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &clBuffer4);
	err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &clBuffer5);
	err = clSetKernelArg(kernel, 5, sizeof(cl_mem), &clBuffer6);
	err = clSetKernelArg(kernel, 6, sizeof(cl_mem), &clBuffer7);
	if (err != CL_SUCCESS) {
		printf("Couldn't set the kernel argument\n");
		exit(1);
	}

	err = clEnqueueNDRangeKernel(queue, kernel, 1, workOffset, workSize, NULL, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Couldn't enqueue the kernel execution command\n");
		exit(1);
	}

	// clean up
	//: cleanup buffers
	clReleaseMemObject(clBuffer1);
	clReleaseMemObject(clBuffer2);
	clReleaseMemObject(clBuffer3);
	clReleaseMemObject(clBuffer4);
	clReleaseMemObject(clBuffer5);
	clReleaseMemObject(clBuffer6);
	clReleaseMemObject(clBuffer7);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseContext(context);
}

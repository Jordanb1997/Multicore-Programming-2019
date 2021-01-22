#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "LoadCL.h"

struct TreeData
{
	TreeData* left;
	int value;
	TreeData* right;

	TreeData(int x) : value(x), left(0), right(0) { }
};

TreeData* add(TreeData* tree, int value)
{
	if (tree == 0)
	{
		return new TreeData(value);
	}
	else if (value < tree->value)
	{
		tree->left = add(tree->left, value);
	}
	else if (value > tree->value)
	{
		tree->right = add(tree->right, value);
	}

	return tree;
}

void print(TreeData* tree)
{
	printf("(");
	if (tree->left) { print(tree->left); printf(" "); }
	printf("%d", tree->value);
	if (tree->right) { printf(" "); print(tree->right); }
	printf(")");
}

int main(int argc, char **argv)
{
	// set a random seed because
	srand(17372);

	TreeData* root = new TreeData(rand() % 4096);

	for (int i = 0; i < 256; i++)
	{
		add(root, rand() % 4096);
	}

	printf("---------------\nCPU-side data\n");
	print(root);
	printf("\n");

	cl_int err;
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem clBuffer1;
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

	//TODO: will probably need only one buffer
	/*
	clBuffer1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(???), ???, &err);
	if (err != CL_SUCCESS) {
		printf("Couldn't create a bufferIn1 object\n");
		exit(1);
	}*/

	//TODO: will need to pass the buffer through
	/*
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuffer1);
	if (err != CL_SUCCESS) {
		printf("Couldn't set the kernel argument\n");
		exit(1);
	}*/

	err = clEnqueueNDRangeKernel(queue, kernel, 1, workOffset, workSize, NULL, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Couldn't enqueue the kernel execution command\n");
		exit(1);
	}

	// clean up
	//TODO: cleanup buffer
	//clReleaseMemObject(clBuffer1);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseContext(context);
}

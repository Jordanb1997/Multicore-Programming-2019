#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

// size of the buffer
const unsigned int BUFFER_SIZE = 1000000;

// buffer
float buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
	cl_int err;
	cl_int err2;
	cl_int err3;


	cl_platform_id platform;

	cl_device_type device_type = CL_DEVICE_TYPE_GPU;
	cl_uint num_entries = 1;
	cl_device_id device_id;
	cl_uint num_devices;

	//context


	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS)
	{
		printf("\nError calling clGetPlatformIDs. Error code: %d\n", err);
		exit(1);
	}

	err2 = clGetDeviceIDs(platform, device_type, num_entries, &device_id, &num_devices);
	if (err2 != CL_SUCCESS)
	{
		printf("\nError calling clGetDeviceIDs. Error code: %d\n", err2);
		exit(2);
	}


	// set a random seed because
	srand(17372);

	// intialise the buffer with some random stuff and output some elements
	for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = rand() / 1000.0f;
	for (int i = 0; i < BUFFER_SIZE; i += BUFFER_SIZE / 7) printf("%10f", buffer[i]);
	printf("\n");

	//TODO: comment out these loops
	// perform a trivial calculation and output some array elements
	for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] *= 1.75f;
	for (int i = 0; i < BUFFER_SIZE; i += BUFFER_SIZE / 7) printf("%10f", buffer[i]);
	printf("\n");

	//TODO: all the OpenCL things
}

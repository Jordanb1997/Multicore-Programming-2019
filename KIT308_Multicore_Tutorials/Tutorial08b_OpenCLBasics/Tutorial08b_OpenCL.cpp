#include <stdio.h>
#include <stdlib.h>
#pragma warning(disable: 4996) // so clCreateCommandQueue still works
#include <CL/cl.h>
#include "LoadCL.h"

// size of the buffer
const unsigned int BUFFER_SIZE_SQRT = 1000;
const unsigned int BUFFER_SIZE = BUFFER_SIZE_SQRT * BUFFER_SIZE_SQRT;

// buffer
float bufferIn1[BUFFER_SIZE];
float bufferIn2[BUFFER_SIZE];
float bufferOut[BUFFER_SIZE];

int main(int argc, char **argv)
{
	// set a random seed because
	srand(17372);

	// intialise the buffer with some random stuff and output some elements
	for (int i = 0; i < BUFFER_SIZE; i++) bufferIn1[i] = rand() / 1000.0f;
	for (int i = 0; i < BUFFER_SIZE; i++) bufferIn2[i] = rand() / 1000.0f;
	for (int i = 0; i < BUFFER_SIZE; i += BUFFER_SIZE / 7) printf("%10f", bufferIn1[i]);
	printf("\n");
	for (int i = 0; i < BUFFER_SIZE; i += BUFFER_SIZE / 7) printf("%10f", bufferIn2[i]);
	printf("\n");

	//TODO: comment out these loops
	// perform a pretty trivial calculation and output some array elements
	for (int i = 0; i < BUFFER_SIZE_SQRT; i++) {
		for (int j = 0; j < BUFFER_SIZE_SQRT; j++) {
			bufferOut[i * BUFFER_SIZE_SQRT + j] = bufferIn1[i * BUFFER_SIZE_SQRT + j] + bufferIn2[i * BUFFER_SIZE_SQRT + j];
		}
	}
	for (int i = 0; i < BUFFER_SIZE; i += BUFFER_SIZE / 7) printf("%10f", bufferOut[i]);
	printf("\n");

	//TODO: all the OpenCL things
}

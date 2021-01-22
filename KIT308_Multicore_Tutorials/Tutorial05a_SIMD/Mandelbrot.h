#ifndef _MANDELBROT_H
#define _MANDELBROT_H

// Mandelbrot defaults and limits
#define DEFAULT_THREADS 1

#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096
#define MAX_COLOURS 256				// must be a power of 2
#define DEFAULT_WIDTH 4			// no bigger than MAX_WIDTH
#define DEFAULT_HEIGHT 4			// no bigger than MAX_HEIGHT
#define DEFAULT_SAMPLES 1

#define DEFAULT_MAX_ITER 32
#define DEFAULT_CENTRE_X -0.75f;
#define DEFAULT_CENTRE_Y 0.0f;
#define DEFAULT_SCALE_Y 0.5f;

void mandelbrot(unsigned int threadID, unsigned int iterations, float centrex, float centrey, float scaley, 
	unsigned int samples, unsigned int width, unsigned int height, unsigned int* out, unsigned int* lineCount);

#endif

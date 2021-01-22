#include <windows.h>
#include <stdio.h>
#include <immintrin.h>
#include "Mandelbrot.h"

// convert iterations to colour information
inline unsigned int iterations2colour(unsigned int iter, unsigned int max_iter, unsigned int flags)
{
	// bound iterations to number of available colours
	iter = (iter * MAX_COLOURS / max_iter) & (MAX_COLOURS - 1);

	// convert iterations to colour scale based on flags (7 basic colour scales possible)
	return (((flags & 4) << 14) | ((flags & 2) << 7) | (flags & 1)) * iter;
}

// calculate Mandelbrot set
void mandelbrot(unsigned int threadID, unsigned int iterations, float centrex, float centrey, float scaley,
	unsigned int samples, unsigned int width, unsigned int height, unsigned int* out, unsigned int* lineCount)
{
	// calculate the x distance to render based on aspect ratio of desired image and y-scale given
	float scalex = scaley * width / height;

	//TODO: STEP 2: add a SIMD vector variable to hold a SIMD-ified version of samples 

	//TODO: STEP 2: replace these declarations with appropriate SIMD vector types 
	// calculate step size for x- and y-axis on the complex plane
	float dx = scalex / width / samples;
	float dy = scaley / height / samples;

	//TODO: STEP 2: replace these declarations with appropriate SIMD vector types 
	// calculate top-left position on the complex plane
	float startx = centrex - scalex * 0.5f;
	float starty = centrey - scaley * 0.5f;

	// potentially loop through entire image size (ie. (0,0) to (width - 1, height - 1)), doing a single row at a time
	unsigned int iy;

	while ((iy = InterlockedIncrement(lineCount)) < height)
	{
		const int SIMD_WIDTH = 4;

		for (unsigned int ix = 0; ix < width; ix += SIMD_WIDTH)
		{
			__m128i totalCalcs = _mm_set1_epi32(0);

			for (unsigned int aay = 0; aay < samples; aay++)
			{
				for (unsigned int aax = 0; aax < samples; aax++)
				{
					__m128i iters = _mm_set1_epi32(0);

					//TODO: STEP 2: use SIMD versions of dx, dy, startx, and starty instead of SIMD-ify scalar ones
					//TODO: STEP 1: declare SIMD vector versions of x0, y0, x, and y here
					__m128 simdx = _mm_setr_ps(0,1,2,3);
					__m128 x0 = startx + ((ix + simdx) * samples + aax) * dx;
					__m128 y0 = starty + (iy * samples + aay) * dy;
					__m128 x;
					__m128 y;
					//TODO: STEP 1: initialise their values being careful to:
					//				- use the correct intrinsics for multiplication and conversion
					//				- correctly calculate four different values for x0 in the SIMD vector

					for (unsigned int simdx = 0; simdx < SIMD_WIDTH; simdx++)
					{
						//TODO: STEP 1: remove the declarations of x0, y0, x, and y are replace with SIMD versions (as above)

						float x0 = ix;
						x0 += simdx;  // dont do this for y just x
						x0 *= samples;
						x0 += aax;
						x0 *= dx;
						x0 += startx;
						// calculate location on the complex plane to render for the current pixel
						float y0 = starty + (iy * samples + aay) * dy;

						// initialise complex number z represented in x and y (ie. z = (x + yi)) 
						// to be current location in complex plane 
						float x = x0;
						float y = y0;

						/*if (simdx == 0) printf("x0:");
						printf(" %f", x0s.m128_f32[simdx]);
						if (simdx == SIMD_WIDTH - 1) printf(", y0: %f\n", y0s.m128_f32[0]);*/

						// iterate complex formula z = z^2 + c (c = starting location in complex plane)
						// until maximum iterations reached or |z| (distance from origin) is greater than 2
						// z^2 = (x + yi)*(x + yi) = x*x + 2*x*y*i + y*y*i*i = (x*x - y*y) + (2*x*y)i

						//TODO: STEP 1: replace all uses of x, y, x0, and y0 with corresponding accesses to new SIMD vectors
						while (x*x + y*y < (2 * 2) && iters.m128i_u32[simdx] <= iterations)
						{
							float xtemp = x*x - y*y + x0;

							y = 2 * x*y + y0;
							x = xtemp;
							iters.m128i_u32[simdx] += 1;
						}
					}

					// if point has escaped add the number of iterations it took to do so to the running total (i.e. stable points don't add to the total)
					__m128i itersGtIterations = _mm_cmpgt_epi32(iters, _mm_set1_epi32(iterations));
					__m128i itersGtIterationsRHS = _mm_or_si128(_mm_andnot_si128(itersGtIterations, iters), _mm_and_si128(itersGtIterations, _mm_set1_epi32(0)));
					totalCalcs = _mm_add_epi32(totalCalcs, itersGtIterationsRHS);

					/*for (unsigned int simdx = 0; simdx < SIMD_WIDTH; simdx++)
					{
						printf("%3d", totalCalcs.m128i_u32[simdx]);
					}
					printf("\n");*/
				}
			}

			// convert number of iterations to colour and store in buffer
			for (unsigned int simdx = 0; simdx < SIMD_WIDTH; simdx++)
			{
				out[iy * width + ix + simdx] = iterations2colour(totalCalcs.m128i_i32[simdx] / (samples * samples), iterations, threadID % 7 + 1);
			}
		}
	}
}

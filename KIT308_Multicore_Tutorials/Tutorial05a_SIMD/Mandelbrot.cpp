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

	// calculate step size for x- and y-axis on the complex plane
	float dx = scalex / width / samples;
	float dy = scaley / height / samples;

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
			__m128i totalCalcs = _mm_setr_epi32(0,0,0,0);

			for (unsigned int aay = 0; aay < samples; aay++)
			{
				for (unsigned int aax = 0; aax < samples; aax++)
				{
					// STEP 2: add a new variable iters of an appropriate SIMD vector type to replace iter (below)
					__m128i iters = _mm_set1_epi32(0);
					for (unsigned int simdx = 0; simdx < SIMD_WIDTH; simdx++)
					{
						// STEP 2: remove/comment out this declaration
						//unsigned int iter = 0;

						// calculate location on the complex plane to render for the current pixel
						float x0 = startx + ((ix + simdx) * samples + aax) * dx;
						float y0 = starty + (iy * samples + aay) * dy;

						// initialise complex number z represented in x and y (ie. z = (x + yi)) 
						// to be current location in complex plane 
						float x = x0;
						float y = y0;

						// iterate complex formula z = z^2 + c (c = starting location in complex plane)
						// until maximum iterations reached or |z| (distance from origin) is greater than 2
						// z^2 = (x + yi)*(x + yi) = x*x + 2*x*y*i + y*y*i*i = (x*x - y*y) + (2*x*y)i
						while (x*x + y*y < (2 * 2) && iters.m128i_i32[simdx] <= iterations)
						{
							float xtemp = x*x - y*y + x0;

							y = 2 * x*y + y0;
							x = xtemp;
							iters.m128i_i32[simdx] += 1;
						}

						//: STEP 2: move this line outside of the simdx loop
						//: STEP 1: replace this array access with one via the SIMD union type
						
					}
					__m128i iterate = _mm_set1_epi32(iterations);
					__m128i zero = _mm_setzero_si128();
					__m128i is_gt = _mm_cmpgt_epi32(iters, iterate);
					__m128i rhs = _mm_or_si128(_mm_and_si128(is_gt, zero), _mm_andnot_si128(is_gt, iters));
					totalCalcs = _mm_add_epi32(totalCalcs, rhs);

					for (unsigned int simdx = 0; simdx < SIMD_WIDTH; simdx++)
					{
						//if (iters.m128i_i32[simdx] <= iterations) totalCalcs.m128i_i32[simdx] += iters.m128i_i32[simdx];

						
						printf("ITERS:::::%d\n", iters.m128i_i32[simdx]);
					}
					// STEP 4: rewrite this loop with conditional expressions as a sequence of SIMD instructions with the form (pseudocode):
					//              - is_gt = cmpgt(iters, iterations)
					//				- rhs = select(is_gt, 0, iters)
					//				- totalCalcs = totalCalcs + rhs
					//: STEP 3: add printf statements to this loop (for values of iters) and check the output
					//: STEP 2: write another simdx loop to perform the above calculation using iters
				}
			}

			// convert number of iterations to colour and store in buffer
			for (unsigned int simdx = 0; simdx < SIMD_WIDTH; simdx++)
			{
				//STEP 1: replace the totalCalcs array access with one via the SIMD union type
				out[iy * width + ix + simdx] = iterations2colour(totalCalcs.m128i_i32[simdx] / (samples * samples), iterations, threadID % 7 + 1);
			}
		}
	}
}


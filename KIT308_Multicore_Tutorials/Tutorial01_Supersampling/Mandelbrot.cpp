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
void mandelbrot(unsigned int samples, unsigned int iterations, float centrex, float centrey, float scaley, 
	unsigned int width, unsigned int height, unsigned int* out)
{
	// calculate the x distance to render based on aspect ratio of desired image and y-scale given
	float scalex = scaley * width / height;

	// calculate step size for x- and y-axis on the complex plane
	float dx = (scalex / width)/samples;
	float dy = (scaley / height)/samples;

	// calculate top-left position on the complex plane
	float startx = centrex - scalex * 0.5f;
	float starty = centrey - scaley * 0.5f;

	// loop through entire image size (ie. (0,0) to (width - 1, height - 1))
	// and through complex plane (ie. (minx, miny) to (maxx - dx, maxy - dy))
	for (unsigned int iy = 0; iy < height; iy += 1)
	{
		for (unsigned int ix = 0; ix < width; ix += 1)
		{
			
			unsigned int iter_total = 0;
			unsigned int iter = 0;

			for (unsigned int sx = 0; sx < samples; sx += 1)
			{
				for (unsigned int sy = 0; sy < samples; sy += 1)
				{
					// calculate location on the complex plane to render for the current pixel
					float x0 = startx + ix * dx *samples +sx * dx;
					float y0 = starty + iy * dy *samples +sy *dy;

					// initialise complex number z represented in x and y (ie. z = (x + yi)) 
					// to be current location in complex plane 
					float x = x0;
					float y = y0;

					// iterate complex formula z = z^2 + c (c = starting location in complex plane)
					// until maximum iterations reached or |z| (distance from origin) is greater than 2
					// z^2 = (x + yi)*(x + yi) = x*x + 2*x*y*i + y*y*i*i = (x*x - y*y) + (2*x*y)i
					while (x*x + y * y < (2 * 2) && iter <= iterations)
					{
						float xtemp = x * x - y * y + x0;

						y = 2 * x*y + y0;
						x = xtemp;
						iter += 1;
					}
					if (!(iter >= iterations))
					{
						iter_total += iter;
					}
				}
			}
			// convert number of iterations to colour and store in buffer
			*out++ = iterations2colour(iter_total, (samples*samples), 7);
		}
	}
}


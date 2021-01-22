/*  The following code is a VERY heavily modified from code originally sourced from:
Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#define TARGET_WINDOWS

#pragma warning(disable: 4996)
#include "Timer.h"
#include "Primitives.h"
#include "Scene.h"
#include "Lighting.h"
#include "Intersection.h"
#include "ImageIO.h"

unsigned int buffer[MAX_WIDTH * MAX_HEIGHT];

// ThreadData structure to hold all the parameters necessary for running the mandelbrot function
//holds: the scene itself, the current threadID, total number of threads, width, height of image, number of AA samples, a pointer to the output image
struct ThreadData
{
	//contains data necessary for each thread
	Scene scene;
	unsigned int threadID;
	int threadTotal;
	unsigned int width;
	int height;
	unsigned int samples;
	unsigned int blockSize;
	unsigned int *out;
	unsigned int *blockCount;
};

// reflect the ray from an object
Ray calculateReflection(const Ray* viewRay, const Intersection* intersect)
{
	// reflect the viewRay around the object's normal
	Ray newRay = { intersect->pos, viewRay->dir - (intersect->normal * intersect->viewProjection * 2.0f) };

	return newRay;
}


// refract the ray through an object
Ray calculateRefraction(const Ray* viewRay, const Intersection* intersect, float* currentRefractiveIndex)
{
	// change refractive index depending on whether we are in an object or not
	float oldRefractiveIndex = *currentRefractiveIndex;
	*currentRefractiveIndex = intersect->insideObject ? DEFAULT_REFRACTIVE_INDEX : intersect->material->density;

	// calculate refractive ratio from old index and current index
	float refractiveRatio = oldRefractiveIndex / *currentRefractiveIndex;

	// Here we take into account that the light movement is symmetrical from the observer to the source or from the source to the oberver.
	// We then do the computation of the coefficient by taking into account the ray coming from the viewing point.
	float fCosThetaT;
	float fCosThetaI = fabsf(intersect->viewProjection);

	// glass-like material, we're computing the fresnel coefficient.
	if (fCosThetaI >= 1.0f)
	{
		// In this case the ray is coming parallel to the normal to the surface
		fCosThetaT = 1.0f;
	}
	else
	{
		float fSinThetaT = refractiveRatio * sqrtf(1 - fCosThetaI * fCosThetaI);

		// Beyond the angle (1.0f) all surfaces are purely reflective
		fCosThetaT = (fSinThetaT * fSinThetaT >= 1.0f) ? 0.0f : sqrtf(1 - fSinThetaT * fSinThetaT);
	}

	// Here we compute the transmitted ray with the formula of Snell-Descartes
	Ray newRay = { intersect->pos, (viewRay->dir + intersect->normal * fCosThetaI) * refractiveRatio - (intersect->normal * fCosThetaT) };

	return newRay;
}


// follow a single ray until it's final destination (or maximum number of steps reached)
Colour traceRay(const Scene* scene, Ray viewRay)
{
	Colour output(0.0f, 0.0f, 0.0f); 								// colour value to be output
	float currentRefractiveIndex = DEFAULT_REFRACTIVE_INDEX;		// current refractive index
	float coef = 1.0f;												// amount of ray left to transmit
	Intersection intersect;											// properties of current intersection

																	// loop until reached maximum ray cast limit (unless loop is broken out of)
	for (int level = 0; level < MAX_RAYS_CAST; ++level)
	{
		// check for intersections between the view ray and any of the objects in the scene
		// exit the loop if no intersection found
		if (!objectIntersection(scene, &viewRay, &intersect)) break;

		// calculate response to collision: ie. get normal at point of collision and material of object
		calculateIntersectionResponse(scene, &viewRay, &intersect);

		// apply the diffuse and specular lighting 
		if (!intersect.insideObject) output += coef * applyLighting(scene, &viewRay, &intersect);

		// if object has reflection or refraction component, adjust the view ray and coefficent of calculation and continue looping
		if (intersect.material->reflection)
		{
			viewRay = calculateReflection(&viewRay, &intersect);
			coef *= intersect.material->reflection;
		}
		else if (intersect.material->refraction)
		{
			viewRay = calculateRefraction(&viewRay, &intersect, &currentRefractiveIndex);
			coef *= intersect.material->refraction;
		}
		else
		{
			// if no reflection or refraction, then finish looping (cast no more rays)
			return output;
		}
	}

	// if the calculation coefficient is non-zero, read from the environment map
	if (coef > 0.0f)
	{
		Material& currentMaterial = scene->materialContainer[scene->skyboxMaterialId];

		output += coef * currentMaterial.diffuse;
	}

	return output;
}


// render scene at given width and height and anti-aliasing level
void render(Scene* scene, unsigned int threadID, int threadTotal, const int width, const int height, /*unsigned int buffer,*/ const int aaLevel, unsigned int blockSize, unsigned int* out, unsigned int* blockCount)
{
	//holding thread ID, and the size of the blocks
	int threadnum = threadID;
	int blocks = blockSize;

	//maintaining image segments for thread height
	int heightStart = height / 2; //the base height
	int starty = height / threadTotal; //height of each thread
	int thready = starty * (threadnum + 1); //calculate the height of the current thread

	//maintaining image segments for thread width
	int widthStart = width / 2;
	int startx = width / threadTotal;
	int threadx = startx * (threadnum + 1);

	//image width and height in blocks
	unsigned int widthBlocks = width / blockSize;
	unsigned int heightBlocks = height / blockSize;
	//curent block
	unsigned int blockNum;

	// angle between each successive ray cast (per pixel, anti-aliasing uses a fraction of this)
	const float dirStepSize = 1.0f / (0.5f * width / tanf(PIOVER180 * 0.5f * scene->cameraFieldOfView));

	while((blockNum = InterlockedIncrement(blockCount))< (widthBlocks*heightBlocks))
	{
		//dealing with the width and height of each block
		int blockx = blockNum % widthBlocks;
		int blocky = blockNum / widthBlocks;

		//converting these from unsigned to signed to avoid type mismatch
		int ylimit = ((blocky + 1)*blocks);
		int xlimit = ((blockx + 1)*blocks);

		//takes the previous varaibles and performs calculation for the image x and y
		int changingy = (heightBlocks / ylimit) + (threadnum+1);
		int changingx = (heightBlocks / xlimit) + (threadnum+1);

		// loop through all the pixels by blocks
		for (int y = (blocky*blocks) + threadnum; y < changingy; ++y)
		{
			//looping through the x coordinate for the blocks
			for (int x = (blockx*blocks)+ threadnum; x < changingx; ++x)
			{
				Colour output(0.0f, 0.0f, 0.0f);

				// calculate multiple samples for each pixel
				const float sampleStep = 1.0f / aaLevel, sampleRatio = 1.0f / (aaLevel * aaLevel);

				// loop through all sub-locations within the pixel
				for (float fragmentx = float(x); fragmentx < x + 1.0f; fragmentx += sampleStep)
				{
					for (float fragmenty = float(y); fragmenty < y + 1.0f; fragmenty += sampleStep)
					{
						// direction of default forward facing ray
						Vector dir = { fragmentx * dirStepSize, fragmenty * dirStepSize, 1.0f };

						// rotated direction of ray
						Vector rotatedDir = {
							dir.x * cosf(scene->cameraRotation) - dir.z * sinf(scene->cameraRotation),
							dir.y,
							dir.x * sinf(scene->cameraRotation) + dir.z * cosf(scene->cameraRotation) };

						// view ray starting from camera position and heading in rotated (normalised) direction
						Ray viewRay = { scene->cameraPosition, normalise(rotatedDir) };

						// follow ray and add proportional of the result to the final pixel colour
						output += sampleRatio * traceRay(scene, viewRay);
					}
				}

				// store saturated final colour value in image buffer
				//stores the next part of the generated image
				out[y*width+x] = output.convertToPixel(scene->exposure);
			}
		}
	}
}

// a THREAD_START_ROUTINE function that casts its argument, calls the render function, and then exits gracefully
DWORD __stdcall renderThreadStart(LPVOID threadData)
{
	// cast the pointer to void (i.e. an untyped pointer) into something we can use
	ThreadData* data = (ThreadData*)threadData;

	render(&data->scene, data->threadID,data->threadTotal, data->width, data->height, /*data->buffer,*/ data->samples, data->blockSize, data->out, data->blockCount);

	ExitThread(NULL);
}

// read command line arguments, render, and write out BMP file
int main(int argc, char* argv[])
{
	int width = 1024;
	int height = 1024;
	int samples = 1;
	
	// rendering options
	int times = 1;
	unsigned int threads = 2;			
	bool colourise = false;	
	unsigned int blockSize = 8;		

	//last block created
	unsigned int blockCount = -1;

	// default input / output filenames
	const char* inputFilename = "../Scenes/cornell.txt";
	char outputFilenameBuffer[1000];
	char* outputFilename = outputFilenameBuffer;

	// do stuff with command line args
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-size") == 0)
		{
			width = atoi(argv[++i]);
			height = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-samples") == 0)
		{
			samples = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-input") == 0)
		{
			inputFilename = argv[++i];
		}
		else if (strcmp(argv[i], "-output") == 0)
		{
			outputFilename = argv[++i];
		}
		else if (strcmp(argv[i], "-runs") == 0)
		{
			times = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-threads") == 0)
		{
			threads = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-colourise") == 0)
		{
			colourise = true;
		}
		else if (strcmp(argv[i], "-blockSize") == 0)
		{
			blockSize = atoi(argv[++i]);
		}
		else
		{
			fprintf(stderr, "unknown argument: %s\n", argv[i]);
		}
	}

	// nasty (and fragile) kludge to make an ok-ish default output filename (can be overriden with "-output" command line option)
	sprintf(outputFilenameBuffer, "../Outputs/%s_%dx%dx%d_%s.bmp", (strrchr(inputFilename, '/') + 1), width, height, samples, (strrchr(argv[0], '\\') + 1));

	// read scene file
	Scene scene;
	if (!init(inputFilename, scene))
	{
		fprintf(stderr, "Failure when reading the Scene file.\n");
		return -1;
	}

	// total time taken to render all runs (used to calculate average)
	int totalTime = 0;
	for (int i = 0; i < times; i++)
	{
		Timer timer;									// create timer

		HANDLE* threadHandles = new HANDLE[threads];
		ThreadData* threadData = new ThreadData[threads];

		// create all the threads with sensible initial values
		for (unsigned int i = 0; i < threads; i++) {

			threadData[i].scene = scene;
			threadData[i].threadID = i;
			threadData[i].threadTotal = threads;
			threadData[i].samples = samples;
			threadData[i].width = width;
			threadData[i].height = height;
			threadData[i].blockSize = blockSize;
			threadData[i].out = buffer + i * width * height / threads;
			threadData[i].blockCount = &blockCount;


			threadHandles[i] = CreateThread(NULL, 0, renderThreadStart, (void*)&threadData[i], 0, NULL);
		}

		// wait for everything to finish
		for (unsigned int i = 0; i < threads; i++) {
			WaitForSingleObject(threadHandles[i], INFINITE);
		}

		// release dynamic memory
		delete[] threadHandles;
		delete[] threadData;

		timer.end();									// record end time
		totalTime += timer.getMilliseconds();
	}

	// output timing information (times run and average)
	printf("average time taken (%d run(s)): %ums\n", times, totalTime / times);

	// output BMP file
	write_bmp(outputFilename, buffer, width, height, width);
}

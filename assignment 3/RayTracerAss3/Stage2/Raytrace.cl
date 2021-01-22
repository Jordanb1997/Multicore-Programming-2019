//type def for colour, vector, point 
typedef float3 Point;
typedef float3 Vector;
typedef float3 Colour;

//////////////////////////////////////
///////the structs that are needed////
//////////////////////////////////////
// rays are cast from a starting point in a direction
typedef struct Ray
{
	Point start;
	Vector dir;
} Ray;
// material
typedef struct Material
{
	// type of colouring/texturing
	enum { GOURAUD, CHECKERBOARD, CIRCLES, WOOD } type;

	Colour diffuse;				// diffuse colour
	Colour diffuse2;			// second diffuse colour, only for checkerboard types

	Vector offset;				// offset of generated texture
	float size;					// size of generated texture

	Colour specular;			// colour of specular lighting
	float power;				// power of specular reflection

	float reflection;			// reflection amount
	float refraction;			// refraction amount
	float density;				// density of material (affects amount of defraction)
} Material;
// sphere object
typedef struct Sphere
{
	Point pos;					// a point on the plane
	float size;					// radius of sphere
	unsigned int materialId;	// material id
} Sphere;
// light object
 typedef struct Light
{
	Point pos;					// location
	Colour intensity;			// brightness and colour
} Light;
// triangle object
typedef struct Triangle
{
	Point p1, p2, p3;			// the three points of the triangle
	Vector normal;				// normal of the triangle
	unsigned int materialId;	// material id
} Triangle;
// all pertinant information about an intersection of a ray with an object
typedef struct Intersection
{
	enum { NONE, SPHERE, TRIANGLE } objectType;			// type of object intersected with

	Point pos;											// point of intersection
	Vector normal;										// normal at point of intersection
	float viewProjection;								// view projection 
	bool insideObject;									// whether or not inside an object

	Material* material;									// material of object
														// object collided with
	union
	{
		__global struct Sphere* sphere;
		__global struct Triangle* triangle;
	};
} Intersection;
// description of a single static scene
__declspec(align(16)) typedef struct Scene
{
	Point cameraPosition;					// camera location
	float cameraRotation;					// direction camera points
	float cameraFieldOfView;				// field of view for the camera

	float exposure;							// image exposure

	unsigned int skyboxMaterialId;

	// scene object counts
	unsigned int numMaterials;
	unsigned int numSpheres;
	unsigned int numTriangles;
	unsigned int numLights;

	// scene objects
	__global struct Material* materialContainer;
	__global struct Sphere* sphereContainer;
	__global struct Triangle* triangleContainer;
	__global struct Light* lightContainer;
} Scene;

unsigned int convertToPixel(Colour c, float exposure)
{
	return ((unsigned char) (255 * (min(1.0f - exp(c.z * exposure), 1.0f))) << 16) +
		((unsigned char) (255 * (min(1.0f - exp(c.y * exposure), 1.0f))) << 8) +
		((unsigned char) (255 * (min(1.0f - exp(c.z * exposure), 1.0f))) << 0);
}
//////////////////////////////////////
///////intersections calculations/////
//////////////////////////////////////

bool isSphereIntersected(struct Sphere s, struct Ray r, float t)
{
	const float EPSILON = 0.01f;

	// Intersection of a ray and a sphere, check the articles for the rationale
	Vector dist = s.pos - r.start;
	float B = dot(r.dir, dist);
	float D = dot(B, B) - dot(dist, dist) + dot(s.size, s.size);

	// if D < 0, no intersection, so don't try and calculate the point of intersection
	if (D < 0.0f) return false;

	// calculate both intersection times(/distances)
	float t0 = B - sqrt(D);
	float t1 = B + sqrt(D);

	// check to see if either of the two sphere collision points are closer than time parameter
	if ((t0 > EPSILON) && (t0 < t))
	{
		t = t0;
		return true;
	}
	else if ((t1 > EPSILON) && (t1 < t))
	{
		t = t1;
		return true;
	}

	return false;
}
void calculateIntersectionResponse(__global struct Scene* scene, struct Ray viewRay, Intersection intersect)
{
	switch (intersect.objectType)
	{
	case SPHERE:
		intersect.normal = normalize(intersect.pos - intersect.sphere->pos);
		//intersect->material = &scene->materialContainer[intersect->sphere->materialId];
		break;
	case TRIANGLE:
		intersect.normal = intersect.triangle->normal;
		//intersect->material = &scene->materialContainer[intersect->triangle->materialId];
		break;
	case NONE:
		break;
	}
	// calculate view projection
	intersect.viewProjection = dot(viewRay.dir, intersect.normal);

	// detect if we are inside an object (needed for refraction)
	intersect.insideObject = (dot(intersect.normal, viewRay.dir) > 0.0f);

	// if inside an object, reverse the normal
	if (intersect.insideObject)
	{
		intersect.normal = dot(intersect.normal, -1.0f);
	}
}



//////////////////////////////////////
///////intersections for rays/////////
//////////////////////////////////////

// test to see if collision between ray and any object in the scene
// updates intersection structure if collision occurs
bool objectIntersection(__global struct Scene* scene, struct Ray viewRay, struct Intersection intersect)
{
	// appropriate constants
	const float MAX_RAY_DISTANCE = 2000000.0f;
	const int MAX_RAYS_CAST = 10;

	// set default distance to be a long long way away
	float t = MAX_RAY_DISTANCE;

	// no intersection found by default
	intersect.objectType = NONE;

	// search for sphere collisions, storing closest one found
	for (unsigned int i = 0; i < scene->numSpheres; ++i)
	{
		if (isSphereIntersected(scene->sphereContainer[i], viewRay, t))
		{
			intersect.objectType = SPHERE;
			intersect.sphere = &scene->sphereContainer[i];
		}
	}
	/*
	// search for triangle collisions, storing closest one found
	for (unsigned int i = 0; i < scene->numTriangles; ++i)
	{
		if (isTriangleIntersected(&scene->triangleContainer[i], viewRay, &t))
		{
			intersect->objectType = TRIANGLE;
			intersect->triangle = &scene->triangleContainer[i];
		}
	}
	*/
	// nothing detected, return false
	if (intersect.objectType == NONE)
	{
		return false;
	}

	// calculate the point of the intersection
	intersect.pos = viewRay.start + viewRay.dir * t;

	return true;
}

Colour traceRay(__global struct Scene* scene, struct Ray viewRay)
{
	// appropriate constants
	const float MAX_RAY_DISTANCE = 2000000.0f;
	const int MAX_RAYS_CAST = 10;

	Colour output= (float3)(0.0f, 0.0f, 0.0f); 						// colour value to be output
	float coef = 1.0f;												// amount of ray left to transmit
	Intersection intersect;											// properties of current intersection

	for (int level = 0; level < MAX_RAYS_CAST; ++level)
	{
		// check for intersections between the view ray and any of the objects in the scene
		// exit the loop if no intersection found
		if (!objectIntersection(scene, viewRay, intersect)) break;

		// calculate response to collision: ie. get normal at point of collision and material of object
		calculateIntersectionResponse(scene, viewRay, intersect);

		if (intersect.objectType == SPHERE)
		{
			output = 1.0f;
			return true;
		}
		if (intersect.objectType == TRIANGLE)
		{
			output = 0.0f;
			return true;
		}
		if (intersect.objectType == NONE)
		{
			output = 0.0f;
			return true;
		}
	}

	// if the calculation coefficient is non-zero, read from the environment map
	if (coef > 0.0f)
	{
		//Material& currentMaterial = scene->materialContainer[scene->skyboxMaterialId];

		//output += coef * currentMaterial.diffuse;
	}

	return output;
}

//////////////////////////////////////
///////performing scene generation////
//////////////////////////////////////
__kernel void raytrace(__global struct Scene* scene, __global struct Material* material, __global struct Sphere* sphere, __global struct Triangle* tri,
	__global struct Light* light, const int width, const int height,  int aaLevel, __global unsigned int* buffer)
{
	unsigned int changeOFwidth = get_global_id(0);
	unsigned int changeOFheight = get_global_id(1);



	//relink container pointers
	scene->materialContainer = material;
	scene->sphereContainer = sphere;
	scene->triangleContainer = tri;
	scene->lightContainer = light;

	
	if(changeOFwidth == 0  && changeOFheight == 0)
	{
		printf("GPU side\n");
		printf("number of triangles:%u\n", scene->numTriangles);
		printf("number of sphere:%u\n", scene->numSpheres);
		printf("number of lights:%u\n", scene->numLights);
		printf("number of materials:%u\n", scene->numMaterials);
		printf("number of camera position: %f %f %f\n", scene->cameraPosition.x, scene->cameraPosition.y, scene->cameraPosition.z);
		printf("number of cam rotation:%f\n", scene->cameraRotation);
		printf("number of camera FOV:%f\n", scene->cameraFieldOfView);
		printf("number of exposure:%f\n", scene->exposure);
		printf("number of skyboxMaterialId:%u\n", scene->skyboxMaterialId);
		printf("---------\n");
	}
	//take care of actual image calculations
	const float PI = 3.14159265358979323846f;
	const float PIOVER180 = 0.017453292519943295769236907684886f;
	const float EPSILON = 0.01f;

	float x;
	float y;

	// angle between each successive ray cast (per pixel, anti-aliasing uses a fraction of this)
	const float dirStepSize = 1.0f / (0.5f * width / tan(PIOVER180 * 0.5f * scene->cameraFieldOfView));

	// loop through all the pixels
	Colour output = (float3)(0.0f, 0.0f, 0.0f);

	// calculate multiple samples for each pixel
	const float sampleStep = 1.0f / aaLevel, sampleRatio = 1.0f / (aaLevel * aaLevel);

	// loop through all sub-locations within the pixel
	for (float fragmentx = x; fragmentx < x + 1.0f; fragmentx += sampleStep)
	{
		for (float fragmenty = y; fragmenty < y + 1.0f; fragmenty += sampleStep)
		{
			// direction of default forward facing ray
			Vector dir = { fragmentx * dirStepSize, fragmenty * dirStepSize, 1.0f };

			// rotated direction of ray
			Vector rotatedDir = {
				dir.x * cos(scene->cameraRotation) - dir.z * sin(scene->cameraRotation),
				dir.y,
				dir.x * sin(scene->cameraRotation) + dir.z * cos(scene->cameraRotation) };

			// view ray starting from camera position and heading in rotated (normalised) direction
			Ray viewRay = { scene->cameraPosition, normalize(rotatedDir) };

			// follow ray and add proportional of the result to the final pixel colour
			output += dot(sampleRatio, traceRay(scene, viewRay));
		}
	}
	// store saturated final colour value in image buffer
	buffer[changeOFwidth *  width + changeOFheight] = convertToPixel(output, scene->exposure);
}
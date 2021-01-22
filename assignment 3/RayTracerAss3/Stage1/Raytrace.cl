//type def for colour, vector, point 
typedef float3 Point;
typedef float3 Vector;
typedef float3 Colour;

// rays are cast from a starting point in a direction
__declspec(align(16)) typedef struct Ray
{
	Point start;
	Vector dir;
} Ray;
// material
__declspec(align(16)) typedef struct Material
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
__declspec(align(16)) typedef struct Sphere
{
	Point pos;					// a point on the plane
	float size;					// radius of sphere
	unsigned int materialId;	// material id
} Sphere;
// light object
__declspec(align(16)) typedef struct Light
{
	Point pos;					// location
	Colour intensity;			// brightness and colour
} Light;
// triangle object
__declspec(align(16)) typedef struct Triangle
{
	Point p1;
	Point p2;
	Point p3;			// the three points of the triangle
	Vector normal;				// normal of the triangle
	unsigned int materialId;	// material id
} Triangle;

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

__kernel void raytrace(__global struct Scene* scene, __global struct Material* material, __global struct Sphere* sphere, __global struct Triangle* tri,
	__global struct Light* light, const int width, const int height,  int aalevel, __global unsigned int* buffer)
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

	Colour imColour = (float3)(0.0f, 1.0f, 0.0f);
	buffer[changeOFwidth *  width + changeOFheight] = convertToPixel(imColour, scene->exposure);
}
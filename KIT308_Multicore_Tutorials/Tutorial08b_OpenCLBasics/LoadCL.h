#ifndef _LOADCL_H
#define _LOADCL_H

#include <CL/cl.h>

cl_program clLoadSource(cl_context context, char* filename);

#endif
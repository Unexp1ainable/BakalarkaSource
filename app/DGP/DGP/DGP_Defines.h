#pragma once

#define OUTPUT_INFO
#define PATCHNUM 4
//#define MAXSTEP 100
#define MODEL_PATH "Data\\"
#define ANGLE_THRESHOLD 0.0001
#define OMEGA 0.001
#define NORM_DIFF 0.05
#define MAX_GAUSSSEIDEL 20

#define CROSS(dest,v1,v2) \
	      dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
		  dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

typedef void DGP_VOID; 
typedef char DGP_CHAR;
typedef int DGP_INT;
typedef bool DGP_BOOL;
typedef double DGP_FLOAT;

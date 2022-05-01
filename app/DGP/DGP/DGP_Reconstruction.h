#pragma once
#include <ctime>
#include <stdio.h>
#include <math.h>

#include "DGP_Defines.h"
#include "DGPObList.h"
#include "DGP_Patch.h"
#include "..\LinearSolver\LinearSolver.h"

enum SOLVER_TYPE{ // linear equation solver type
	NONE_TYPE,    // current only support direct solver 
	DIRECT_SOLVER,// and Gauss seidel CPU implementation
	GAUSS_SEIDEL_CPU,
	GAUSS_SEIDEL_GPU,
	TOTAL_TYPE
};
class DGP_Reconstruction
{
public:
	DGP_Reconstruction();
	virtual ~DGP_Reconstruction();

	DGP_VOID DoComputing(SOLVER_TYPE type = DIRECT_SOLVER, const DGP_INT partNum = 1, const DGP_INT maxStep = 5);// main routine function  
																												 // type: linear solver type
																												 // partNum: connected part number
																												 // maxStep: maximum iterarion steps
	DGPObList* GetPatchList() {return patchList;}; // return patch list
	DGPObList* GetVertexList() {return vertexList;}; // return vertex list
private:
	SOLVER_TYPE n_type; // solver type
	DGP_INT patchNum, vertexNum; // total patch number and vertex number
	DGPObList* patchList, * vertexList; // list of total patch and list of total vertex
	DGPObList* cur_PatchList, * cur_VertexList; // list of patch which are connected and list of vetices which are connected

	DGP_VOID* factorized; // factorized matrix (for direct solver)
	CSpMatrix* matA, * matATA;	// input matrix A which is M X N (M>N) 
								// and its least square ATA which is N X N 

	DGP_FLOAT* vectorR, * vectorB, * vectorX, * vectorX1;	// vectorR: right hand side vector (M X 1)
															// vectorB: right hand side vector which equals to AT X vectorR (N X 1);
															// vectorX: unknown depth for all vertex which is N X 1
															// vectorX1: unknown depth for all vertex which is N X 1 (for Gauss Seidel)
															

	DGP_VOID _setUpCurList(const int curPartNum); // fill cur_PatchList and cur_VertexList according to IdentifiedIndex
	DGP_BOOL _initialization(); // initialize for computing
	DGP_VOID _clearAll(); // delete all memories
	DGP_VOID _resetAllId(); // reset ids for patch and vertex
	DGP_VOID _checkTargetNormalValidity(); // check input normal for all patch
	DGP_VOID _checkTargetNormalEachPatch(DGP_Patch* patch); // check input normal for one patch 
	DGP_VOID _projection(); // do projection for all patch
	DGP_VOID _projectionEachPatch(DGP_Patch* patch); // do projection for one patch
	DGP_VOID _centeringEachPatch(DGP_Patch* patch, const DGP_INT patchId); // for each vertex of one patch, minus the center point of the patch

	DGP_VOID _fillMatrixA(); // fill matA
	DGP_VOID _factorizeMatrixA(); // factorize matATA
	DGP_VOID _fillVectorB(); // fill vectorB
	DGP_VOID _backSubstitution(); // do back substitution
	DGP_VOID _updateResult(); // update result

	DGP_VOID _compareNormal(); // compare current normal vs target normal
	DGP_VOID _computePatchNormal(DGP_Patch* patch, DGP_FLOAT& nx, DGP_FLOAT& ny, DGP_FLOAT& nz);
	DGP_VOID _computePatchNormal_Local(DGP_Patch* patch, DGP_FLOAT& nx, DGP_FLOAT& ny, DGP_FLOAT& nz);

	// Gauss-Seidel CPU
	DGP_FLOAT _norm_diff(); // compute norm difference of resultant depth in previous and current steps
	DGP_VOID _initialize_Gauss_Seidel(); // initialize for Gauss Seidel solver
	DGP_VOID _initialize_intermediateVector(); // initialize for intermediate result
	DGP_VOID _update_intermediateVector(); // update intermediate result
	DGP_VOID _gauss_seidel_CPU(); // do Gauss Seidel interation 
	DGP_VOID _check_exist(const DGP_INT id, DGP_INT index_array[]);
	DGP_VOID _check_exist(DGP_Patch* patch, DGP_Patch* neighborArray[8]);
	DGP_FLOAT _compute_depthDiff(DGP_Patch* centerPatch, DGP_Patch* fixedPatch);
};

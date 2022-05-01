/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  This code is written by Yunbo Zhang (email: will.yunbo.zhang@gmail.com).                           //
//  The implementation is base on following paper:                                                     //
//  Wuyuan Xie, Yunbo Zhang, Charlie C.L. Wang, and Ronald C.-K. Chung,                                //
//  "Surface-from-Gradients: An approach based on discrete geometry processing",                       //
//  2014 IEEE Conference on Computer Vision and Pattern Recognition, Columbus, Ohio, June 24-27, 2014. //                                     
//  Copyright (c) Yunbo Zhang 2014. All rights reserved.                                               //                  
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DGP_FileIO.h"
#include "DGP_Reconstruction.h"

DGP_Reconstruction::DGP_Reconstruction()
{
	patchList = new DGPObList; vertexList = new DGPObList;
	cur_PatchList = new DGPObList; cur_VertexList = new DGPObList;
	vectorB = NULL; vectorX = NULL; vectorX1 = NULL; vectorR = NULL;
	matA = NULL; matATA = NULL; factorized = NULL;
}

DGP_Reconstruction::~DGP_Reconstruction()
{
	_clearAll();
}

DGP_VOID DGP_Reconstruction::DoComputing(SOLVER_TYPE type, const DGP_INT partNum, const DGP_INT maxStep)
{
	n_type = type;
	for (int i = 0; i < partNum; i++) {
		_setUpCurList(i);
		if (!_initialization()) continue;
		switch (n_type) {
		case DIRECT_SOLVER: { // direct solver
			_fillMatrixA();
			_factorizeMatrixA();
			for (DGP_INT i = 0; i < maxStep; i++) {
				_projection();
				_fillVectorB();
				_backSubstitution();
				_updateResult();
				printf("Step %d finished!\n", i + 1);
			}
			if (factorized) {
				LinearSolver::DeleteF(factorized);
				factorized = NULL;
			}
		}break;
		case GAUSS_SEIDEL_CPU: { // Gauss Seidel solver
			_initialize_Gauss_Seidel();
			for (DGP_INT i = 0; i < maxStep; i++) {
				_projection();
				_initialize_intermediateVector();
				_fillVectorB();
				_gauss_seidel_CPU();
				_updateResult();
				printf("Step %d finished!\n", i + 1);
			}
		}break;
		case GAUSS_SEIDEL_GPU: {
		}break;
		}
	}
	_resetAllId();
	_compareNormal();
}

DGP_VOID DGP_Reconstruction::_setUpCurList(const int curPartNum)
{
	cur_PatchList->RemoveAll(); cur_VertexList->RemoveAll();
	for (DGPPOSITION Pos = patchList->GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(Pos);
		if (patch->GetIdentifiedIndex() == (curPartNum + 1)) {
			cur_PatchList->AddTail(patch);
		}
	}
	for (DGPPOSITION Pos = vertexList->GetHeadPosition(); Pos != NULL;) {
		DGP_Vertex* vertex = (DGP_Vertex*)vertexList->GetNext(Pos);
		if (vertex->GetIdentifiedIndex() == (curPartNum + 1)) {
			cur_VertexList->AddTail(vertex);
		}
	}
}

DGP_BOOL DGP_Reconstruction::_initialization()
{
	patchNum = vertexNum = 0;
	patchNum = cur_PatchList->GetCount();

	//DGPPOSITION Pos=cur_VertexList->GetTailPosition();
	//DGP_Vertex* vertex = (DGP_Vertex*)cur_VertexList->GetPrev(Pos);
	//vertexNum = cur_VertexList->GetCount()-2;
	//vertex->SetIndexNo(-1);
	//for(;Pos!=NULL;vertexNum--){
	//	vertex = (DGP_Vertex*)cur_VertexList->GetPrev(Pos);
	//	vertex->SetIndexNo(vertexNum);
	//}
	//vertexNum = cur_VertexList->GetCount()-1;

	DGPPOSITION Pos = cur_VertexList->GetHeadPosition();
	DGP_Vertex* vertex = (DGP_Vertex*)cur_VertexList->GetNext(Pos);
	vertex->SetIndexNo(-1); // fix the first vertex in vertex list
	for (; Pos != NULL; vertexNum++) {
		vertex = (DGP_Vertex*)cur_VertexList->GetNext(Pos);
		vertex->SetIndexNo(vertexNum);
	}

	if ((patchNum == 0) || (vertexNum == 0)) return false;
	vectorX = new DGP_FLOAT[vertexNum];
	vectorR = new DGP_FLOAT[patchNum * PATCHNUM];
	if (n_type == DIRECT_SOLVER) {
		vectorB = new DGP_FLOAT[vertexNum];
		matA = new CSpMatrix(patchNum * PATCHNUM, vertexNum);
		matATA = new CSpMatrix(vertexNum, vertexNum);
	}
	else {
		vectorX1 = new DGP_FLOAT[vertexNum];
	}
	_checkTargetNormalValidity();
	return true;
}

DGP_VOID DGP_Reconstruction::_clearAll()
{
	if (matA) {
		delete matA; matA = NULL;
	}
	if (matATA) {
		delete matATA; matATA = NULL;
	}
	if (factorized) {
		LinearSolver::DeleteF(factorized);
		factorized = NULL;
	}
	if (vectorB) {
		delete[]vectorB; vectorB = NULL;
	}
	if (vectorR) {
		delete[]vectorR; vectorR = NULL;
	}
	if (vectorX) {
		delete[]vectorX; vectorX = NULL;
	}
	if (vectorX1) {
		delete[]vectorX1; vectorX1 = NULL;
	}
	if (patchList) {
		for (DGPPOSITION Pos = patchList->GetHeadPosition(); Pos != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(Pos);
			delete patch; patch = NULL;
		}
		patchList->RemoveAll(); delete patchList; patchList = NULL;
	}

	if (vertexList) {
		for (DGPPOSITION Pos = vertexList->GetHeadPosition(); Pos != NULL;) {
			DGP_Vertex* vertex = (DGP_Vertex*)vertexList->GetNext(Pos);
			delete vertex; vertex = NULL;
		}
		vertexList->RemoveAll(); delete vertexList; vertexList = NULL;
	}

	if (cur_PatchList) {
		cur_PatchList->RemoveAll(); delete cur_PatchList; cur_PatchList = NULL;
	}

	if (cur_VertexList) {
		cur_VertexList->RemoveAll(); delete cur_VertexList; cur_VertexList = NULL;
	}
}

DGP_VOID DGP_Reconstruction::_resetAllId()
{
	DGP_INT i = 1;
	for (DGPPOSITION Pos = vertexList->GetHeadPosition(); Pos != NULL; i++) {
		DGP_Vertex* vertex = (DGP_Vertex*)vertexList->GetNext(Pos);
		vertex->SetIndexNo(i);
	}
	i = 1;
	for (DGPPOSITION Pos = patchList->GetHeadPosition(); Pos != NULL; i++) {
		DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(Pos);
		patch->SetIndexNo(i);
	}
}

DGP_VOID DGP_Reconstruction::_checkTargetNormalValidity()
{
	for (DGPPOSITION Pos = cur_PatchList->GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)cur_PatchList->GetNext(Pos);
		_checkTargetNormalEachPatch(patch);
	}
}

DGP_VOID DGP_Reconstruction::_checkTargetNormalEachPatch(DGP_Patch* patch)
{
	DGP_FLOAT n[3];
	patch->GetTargetNormal(n[0], n[1], n[2]);
	patch->SetExcludedFlag(false);
	if (fabs(n[2]) <= ANGLE_THRESHOLD) {
		patch->SetExcludedFlag(true);
	}
}

DGP_VOID DGP_Reconstruction::_projection()
{
	for (DGPPOSITION Pos = cur_PatchList->GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)cur_PatchList->GetNext(Pos);
		_projectionEachPatch(patch);
	}
}

DGP_VOID DGP_Reconstruction::_projectionEachPatch(DGP_Patch* patch)
{
	if (!patch->GetExcludedFlag()) {
		DGP_FLOAT nn[3], center[3], pp[3];
		for (DGP_INT i = 0; i < 3; i++) center[i] = 0.0;
		patch->GetTargetNormal(nn[0], nn[1], nn[2]);
		for (DGP_INT i = 0; i < PATCHNUM; i++) {
			patch->GetVertex(i)->GetPosition(pp[0], pp[1], pp[2]);
			center[0] += pp[0]; center[1] += pp[1]; center[2] += pp[2];
		}
		center[0] /= (DGP_FLOAT)PATCHNUM;
		center[1] /= (DGP_FLOAT)PATCHNUM;
		center[2] /= (DGP_FLOAT)PATCHNUM;

		for (DGP_INT i = 0; i < PATCHNUM; i++) {
			patch->GetVertex(i)->GetPosition(pp[0], pp[1], pp[2]);
			pp[2] = (-((pp[0] - center[0]) * nn[0] + (pp[1] - center[1]) * nn[1]) / nn[2]) + center[2];
			patch->SetLocalPosition(i, pp[0], pp[1], pp[2]);
		}
	}
	else {
		DGP_FLOAT pp[3];
		for (DGP_INT i = 0; i < PATCHNUM; i++) {
			patch->GetVertex(i)->GetPosition(pp[0], pp[1], pp[2]);
			patch->SetLocalPosition(i, pp[0], pp[1], pp[2]);
		}
	}
}

DGP_VOID DGP_Reconstruction::_centeringEachPatch(DGP_Patch* patch, const DGP_INT patchId)
{
	DGP_FLOAT pp[3], center[3];
	for (DGP_INT i = 0; i < 3; i++) center[i] = 0.0;
	for (DGP_INT i = 0; i < PATCHNUM; i++) {
		patch->GetLocalPosition(i, pp[0], pp[1], pp[2]);
		center[0] += pp[0]; center[1] += pp[1]; center[2] += pp[2];
	}
	center[0] /= (DGP_FLOAT)PATCHNUM;
	center[1] /= (DGP_FLOAT)PATCHNUM;
	center[2] /= (DGP_FLOAT)PATCHNUM;

	DGP_INT vertexIdArray[PATCHNUM];
	for (DGP_INT i = 0; i < PATCHNUM; i++) vertexIdArray[i] = patch->GetVertex(i)->GetIndexNo();
	for (DGP_INT i = 0; i < PATCHNUM; i++) {
		patch->GetLocalPosition(i, pp[0], pp[1], pp[2]);
		DGP_FLOAT nx, ny, nz;
		patch->GetTargetNormal(nx, ny, nz);
		nz = (nz + 1)/2;
		vectorR[patchId * PATCHNUM + i] = (pp[2] - center[2])/nz;
		//patch->SetLocalPosition(i, pp[0], pp[1], vectorR[patchId*PATCHNUM+i]);
		for (DGP_INT k = 0; k < PATCHNUM; k++) {
			if (vertexIdArray[k] < 0) {
				patch->GetVertex(k)->GetPosition(pp[0], pp[1], pp[2]);
				if (i == k) {
					vectorR[patchId * PATCHNUM + i] -= (pp[2] * (1.0 - (1.0 / ((DGP_FLOAT)PATCHNUM)))*nz);
				}
				else {
					vectorR[patchId * PATCHNUM + i] -= (pp[2] * (-1.0 / ((DGP_FLOAT)PATCHNUM))*nz);
				}
			}
		}
	}
}

DGP_VOID DGP_Reconstruction::_fillMatrixA()
{
	DGP_INT patchId = 0;
	for (DGPPOSITION Pos = cur_PatchList->GetHeadPosition(); Pos != NULL; patchId++) {
		DGP_Patch* patch = (DGP_Patch*)cur_PatchList->GetNext(Pos);
		DGP_INT vertexIdArray[PATCHNUM];
		for (DGP_INT i = 0; i < PATCHNUM; i++) 
			vertexIdArray[i] = patch->GetVertex(i)->GetIndexNo();

		DGP_FLOAT coeff1 = (-1.0 / ((DGP_FLOAT)PATCHNUM));
		DGP_FLOAT coeff2 = 1.0 + coeff1;

		for (DGP_INT i = 0; i < PATCHNUM; i++) {
			if (vertexIdArray[i] < 0) continue;
			for (DGP_INT j = 0; j < PATCHNUM; j++) {
				if (i == j) {
					matA->Set(patchId * PATCHNUM + j, vertexIdArray[i], coeff2);
				}
				else {
					matA->Set(patchId * PATCHNUM + j, vertexIdArray[i], coeff1);
				}
			}
		}
	}
	CSpMatrix::MulATA(*matA, *matATA);
}

DGP_VOID DGP_Reconstruction::_factorizeMatrixA()
{
	if (!LinearSolver::FactorA(*matATA, factorized)) printf("Cannot factorize Matrix!\n");
}

DGP_VOID DGP_Reconstruction::_fillVectorB()
{
	DGP_INT patchId = 0;
	for (DGPPOSITION Pos = cur_PatchList->GetHeadPosition(); Pos != NULL; patchId++) {
		DGP_Patch* patch = (DGP_Patch*)cur_PatchList->GetNext(Pos);
		_centeringEachPatch(patch, patchId);
	}
	if (n_type == DIRECT_SOLVER) {
		CSpMatrix::MulATB(*matA, vectorR, vectorB);
	}
}

DGP_VOID DGP_Reconstruction::_backSubstitution()
{
	if (!LinearSolver::SolveX(factorized, vectorX, vectorB)) printf("Cannot solve!\n");
}

DGP_VOID DGP_Reconstruction::_updateResult()
{
	DGP_FLOAT pp[3];
	for (DGPPOSITION Pos = cur_VertexList->GetHeadPosition(); Pos != NULL;) {
		DGP_Vertex* vertex = (DGP_Vertex*)cur_VertexList->GetNext(Pos);
		DGP_INT vertexId = vertex->GetIndexNo();
		if (vertexId < 0) continue;
		vertex->GetPosition(pp[0], pp[1], pp[2]);
		vertex->SetPosition(pp[0], pp[1], vectorX[vertexId]);
	}
}

//Gauss-Seidel CPU version
DGP_VOID DGP_Reconstruction::_gauss_seidel_CPU()
{
	DGP_FLOAT omega1 = 1.0 - OMEGA;
	DGP_FLOAT omega2 = OMEGA;
	DGP_FLOAT c1 = (-1.0 / ((DGP_FLOAT)PATCHNUM));
	DGP_FLOAT c2 = 1.0 + c1;
	int steps = 0;
	while (true) {
		_update_intermediateVector();
		for (DGPPOSITION Pos = cur_VertexList->GetHeadPosition(); Pos != NULL;) {
			DGP_Vertex* vertex = (DGP_Vertex*)cur_VertexList->GetNext(Pos);
			if (vertex->GetIndexNo() < 0) continue;
			DGP_FLOAT n_neighbor = (DGP_FLOAT)(vertex->GetPatchList().GetCount());
			DGP_FLOAT yi = 0.0;
			DGP_FLOAT aii = c2 * n_neighbor;
			for (DGPPOSITION Pos1 = vertex->GetPatchList().GetHeadPosition(); Pos1 != NULL;) {
				DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos1);
				DGP_INT patchId = patch->GetIndexNo() - 1;
				DGP_INT i;
				for (i = 0; i < PATCHNUM; i++) {
					if (patch->GetVertex(i) == vertex) break;
				}
				for (DGP_INT j = 0; j < PATCHNUM; j++) {
					if (i == j) {
						yi += c2 * vectorR[patchId * PATCHNUM + j];
					}
					else {
						yi += c1 * vectorR[patchId * PATCHNUM + j];
					}
				}
			}
			DGP_FLOAT S = yi / aii;
			DGP_INT indexArray[8];
			DGP_FLOAT valueArray[8];
			for (DGP_INT i = 0; i < 8; i++) {
				indexArray[i] = -1;
				valueArray[i] = 0.0;
			}
			for (DGPPOSITION Pos1 = vertex->GetPatchList().GetHeadPosition(); Pos1 != NULL;) {
				DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos1);
				for (DGP_INT i = 0; i < PATCHNUM; i++) {
					DGP_Vertex* vertex1 = patch->GetVertex(i);
					if ((vertex1->GetIndexNo() < 0) || (vertex1 == vertex)) continue;
					_check_exist(vertex1->GetIndexNo(), indexArray);
				}
			}
			for (DGPPOSITION Pos1 = vertex->GetPatchList().GetHeadPosition(); Pos1 != NULL;) {
				DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos1);
				for (DGP_INT i = 0; i < PATCHNUM; i++) {
					DGP_Vertex* vertex1 = patch->GetVertex(i);
					if ((vertex1->GetIndexNo() < 0) || (vertex1 == vertex)) continue;
					S -= (c1 * vectorX[vertex1->GetIndexNo()]) / aii;
					DGP_INT k;
					for (k = 0; k < 8; k++) {
						if (vertex1->GetIndexNo() == indexArray[k]) break;
					}
					valueArray[k] += c1;
				}
			}
			for (DGP_INT i = 0; i < 8; i++) {
				if (indexArray[i] == -1) break;
			}
			double aa = omega1 * vectorX1[vertex->GetIndexNo()];
			double bb = omega2 * S;
			double cc = omega1 * vectorX1[vertex->GetIndexNo()] + omega2 * S;
			double dd = vectorX1[vertex->GetIndexNo()];
			double ee = cc - dd;
			vectorX[vertex->GetIndexNo()] = omega1 * vectorX1[vertex->GetIndexNo()] + omega2 * S;

		}
		steps++;
		if ((_norm_diff() <= NORM_DIFF) || (steps >= MAX_GAUSSSEIDEL)) break;
	}
}

DGP_VOID DGP_Reconstruction::_check_exist(const DGP_INT id, DGP_INT index_array[])
{
	DGP_INT i;
	for (i = 0; i < 8; i++) {
		if (index_array[i] == id) return;
		if (index_array[i] == -1) break;
	}
	index_array[i] = id;
}

DGP_VOID DGP_Reconstruction::_check_exist(DGP_Patch* patch, DGP_Patch* neighborArray[8])
{
	DGP_INT i;
	for (i = 0; i < 8; i++) {
		if (neighborArray[i] == patch) return;
		if (!neighborArray[i]) break;
	}
	neighborArray[i] = patch;
}

DGP_FLOAT DGP_Reconstruction::_compute_depthDiff(DGP_Patch* centerPatch, DGP_Patch* fixedPatch)
{
	for (DGP_INT i = 0; i < PATCHNUM; i++) {
		DGP_Vertex* vertex = centerPatch->GetVertex(i);
		DGP_FLOAT xx, yy, zz;
		centerPatch->GetLocalPosition(i, xx, yy, zz);
		for (DGPPOSITION Pos = vertex->GetPatchList().GetHeadPosition(); Pos != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos);
			if (patch == fixedPatch) {
				DGP_INT j;
				for (j = 0; j < PATCHNUM; j++) {
					DGP_Vertex* vertex1 = patch->GetVertex(j);
					if (vertex == vertex1) break;
				}
				DGP_FLOAT xx1, yy1, zz1;
				fixedPatch->GetLocalPosition(j, xx1, yy1, zz1);
				DGP_FLOAT dis = zz1 - zz;
				return dis;
			}
		}
	}
	return 0.0;
}

DGP_VOID DGP_Reconstruction::_update_intermediateVector()
{
	for (int i = 0; i < vertexNum; i++) vectorX1[i] = vectorX[i];
}

DGP_VOID DGP_Reconstruction::_initialize_Gauss_Seidel()
{
	_projection();
	for (DGPPOSITION Pos = cur_PatchList->GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)cur_PatchList->GetNext(Pos);
		patch->SetVisitedFlag(false);
		patch->SetFixedFlag(false);
	}

	DGP_Patch* seed = NULL;
	for (DGPPOSITION Pos = cur_VertexList->GetHeadPosition(); Pos != NULL;) {
		DGP_Vertex* vertex = (DGP_Vertex*)cur_VertexList->GetNext(Pos);
		if (vertex->GetIndexNo() >= 0) continue;
		seed = (DGP_Patch*)vertex->GetPatchList().GetHead();
		break;
	}
	if (!seed) return;

	DGPObList list, keepList;
	list.AddTail(seed);
	seed->SetVisitedFlag(true);
	do {
		keepList.RemoveAll();
		for (DGPPOSITION Pos = list.GetHeadPosition(); Pos != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)list.GetNext(Pos);
			patch->SetVisitedFlag(false);
			DGP_Patch* neighborArray[8];
			for (DGP_INT i = 0; i < 8; i++) neighborArray[i] = NULL;
			for (DGP_INT i = 0; i < PATCHNUM; i++) {
				DGP_Vertex* vertex = patch->GetVertex(i);
				for (DGPPOSITION PosPatch = vertex->GetPatchList().GetHeadPosition(); PosPatch != NULL;) {
					DGP_Patch* patch1 = (DGP_Patch*)vertex->GetPatchList().GetNext(PosPatch);
					if (patch1 == patch) continue;
					_check_exist(patch1, neighborArray);
				}
			}
			DGP_INT fixedNum = 0;
			DGP_FLOAT depth_diff = 0.0;
			for (DGP_INT i = 0; i < 8; i++) {
				DGP_Patch* patch1 = neighborArray[i];
				if (!patch1) continue;
				if (patch1->GetFixedFlag()) {
					depth_diff += _compute_depthDiff(patch, patch1);
					fixedNum++;
				}
			}
			if (fixedNum == 0) {
				for (DGP_INT i = 0; i < PATCHNUM; i++) {
					DGP_Vertex* vertex = patch->GetVertex(i);
					if (vertex->GetIndexNo() >= 0) continue;
					DGP_FLOAT xx, yy, zz, xx1, yy1, zz1;
					vertex->GetPosition(xx1, yy1, zz1);
					patch->GetLocalPosition(i, xx, yy, zz);
					depth_diff = zz1 - zz;
				}
			}
			else {
				depth_diff /= (DGP_FLOAT)fixedNum;
			}
			for (DGP_INT i = 0; i < PATCHNUM; i++) {
				DGP_FLOAT xx, yy, zz;
				patch->GetLocalPosition(i, xx, yy, zz);
				zz += depth_diff;
				patch->SetLocalPosition(i, xx, yy, zz);
			}
			patch->SetFixedFlag(true);
		}
		for (DGPPOSITION Pos = list.GetHeadPosition(); Pos != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)list.GetNext(Pos);
			for (DGP_INT i = 0; i < PATCHNUM; i++) {
				DGP_Vertex* vertex = patch->GetVertex(i);
				for (DGPPOSITION PosPatch = vertex->GetPatchList().GetHeadPosition(); PosPatch != NULL;) {
					DGP_Patch* patch1 = (DGP_Patch*)vertex->GetPatchList().GetNext(PosPatch);
					if ((!patch1->GetVisitedFlag()) && (!patch1->GetFixedFlag())) {
						keepList.AddTail(patch1);
						patch1->SetVisitedFlag(true);
					}
				}
			}
		}
		list.RemoveAll(); list.AddTail(&keepList);
	} while (!(list.IsEmpty()));

	for (DGPPOSITION Pos = cur_VertexList->GetHeadPosition(); Pos != NULL;) {
		DGP_Vertex* vertex = (DGP_Vertex*)cur_VertexList->GetNext(Pos);
		if (vertex->GetIndexNo() < 0) continue;
		DGP_FLOAT average = 0.0;
		for (DGPPOSITION Pos1 = vertex->GetPatchList().GetHeadPosition(); Pos1 != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos1);
			DGP_INT i;
			for (i = 0; i < PATCHNUM; i++) {
				if (patch->GetVertex(i) == vertex) break;
			}
			DGP_FLOAT p[3];
			patch->GetLocalPosition(i, p[0], p[1], p[2]);
			average += p[2];
		}
		average /= (DGP_FLOAT)(vertex->GetPatchList().GetCount());
		double xx, yy, zz;
		vertex->GetPosition(xx, yy, zz);
		vertex->SetPosition(xx, yy, average);
	}
}

DGP_VOID DGP_Reconstruction::_initialize_intermediateVector()
{
	for (DGPPOSITION Pos = cur_VertexList->GetHeadPosition(); Pos != NULL;) {
		DGP_Vertex* vertex = (DGP_Vertex*)cur_VertexList->GetNext(Pos);
		if (vertex->GetIndexNo() < 0) continue;
		DGP_FLOAT average = 0.0;
		for (DGPPOSITION Pos1 = vertex->GetPatchList().GetHeadPosition(); Pos1 != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos1);
			DGP_INT i;
			for (i = 0; i < PATCHNUM; i++) {
				if (patch->GetVertex(i) == vertex) break;
			}
			DGP_FLOAT p[3];
			patch->GetLocalPosition(i, p[0], p[1], p[2]);
			average += p[2];
		}
		average /= (DGP_FLOAT)(vertex->GetPatchList().GetCount());
		vectorX[vertex->GetIndexNo()] = average;
	}
}

DGP_FLOAT DGP_Reconstruction::_norm_diff()
{
	if ((!vectorX) || (!vectorX1)) return 0.0;
	DGP_FLOAT norm = 0.0;
	for (DGP_INT i = 0; i < vertexNum; i++) {
		DGP_FLOAT aa = (vectorX[i] - vectorX1[i]) * (vectorX[i] - vectorX1[i]);
		norm += aa;
	}
	norm = sqrt(norm);
	printf("norm is %lf\n", norm);
	return norm;
}

DGP_VOID DGP_Reconstruction::_compareNormal()
{
	DGP_FLOAT error = 0.0;
	for (DGPPOSITION Pos = patchList->GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(Pos);
		DGP_FLOAT n[3], nn[3];
		patch->GetTargetNormal(nn[0], nn[1], nn[2]);
		_computePatchNormal(patch, n[0], n[1], n[2]);
		error += (1.0 - fabs(DOT(n, nn)));
	}
	DGP_FLOAT count = (DGP_FLOAT)patchList->GetCount();
#ifdef OUTPUT_INFO
	printf("average normal error is %lf\n", error / count);
#endif
}

DGP_VOID DGP_Reconstruction::_computePatchNormal(DGP_Patch* patch, DGP_FLOAT& nx, DGP_FLOAT& ny, DGP_FLOAT& nz)
{
	DGP_FLOAT p[PATCHNUM][3];
	nx = ny = nz = 0.0;
	for (DGP_INT i = 0; i < PATCHNUM; i++) patch->GetVertex(i)->GetPosition(p[i][0], p[i][1], p[i][2]);
	for (DGP_INT i = 0; i < PATCHNUM; i++) {
		DGP_FLOAT p1[3], p2[3], p3[3], pt1[3], pt2[3], n[3];
		p1[0] = p[(i - 1 + PATCHNUM) % PATCHNUM][0]; p1[1] = p[(i - 1 + PATCHNUM) % PATCHNUM][1]; p1[2] = p[(i - 1 + PATCHNUM) % PATCHNUM][2];
		p2[0] = p[i][0]; p2[1] = p[i][1]; p2[2] = p[i][2];
		p3[0] = p[(i + 1) % PATCHNUM][0]; p3[1] = p[(i + 1) % PATCHNUM][1]; p3[2] = p[(i + 1) % PATCHNUM][2];
		pt1[0] = p2[0] - p1[0]; pt1[1] = p2[1] - p1[1]; pt1[2] = p2[2] - p1[2];
		pt2[0] = p3[0] - p1[0]; pt2[1] = p3[1] - p1[1]; pt2[2] = p3[2] - p1[2];
		CROSS(n, pt1, pt2);
		DGP_FLOAT rr = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		n[0] /= rr; n[1] /= rr; n[2] /= rr;
		nx += n[0]; ny += n[1]; nz += n[2];
	}
	nx /= (DGP_FLOAT)PATCHNUM; ny /= (DGP_FLOAT)PATCHNUM; nz /= (DGP_FLOAT)PATCHNUM;
	DGP_FLOAT rr = sqrt(nx * nx + ny * ny + nz * nz);
	nx /= rr; ny /= rr; nz /= rr;
}

DGP_VOID DGP_Reconstruction::_computePatchNormal_Local(DGP_Patch* patch, DGP_FLOAT& nx, DGP_FLOAT& ny, DGP_FLOAT& nz)
{
	DGP_FLOAT p[PATCHNUM][3];
	nx = ny = nz = 0.0;
	for (DGP_INT i = 0; i < PATCHNUM; i++) patch->GetLocalPosition(i, p[i][0], p[i][1], p[i][2]);
	for (DGP_INT i = 0; i < PATCHNUM; i++) {
		DGP_FLOAT p1[3], p2[3], p3[3], pt1[3], pt2[3], n[3];
		p1[0] = p[(i - 1 + PATCHNUM) % PATCHNUM][0]; p1[1] = p[(i - 1 + PATCHNUM) % PATCHNUM][1]; p1[2] = p[(i - 1 + PATCHNUM) % PATCHNUM][2];
		p2[0] = p[i][0]; p2[1] = p[i][1]; p2[2] = p[i][2];
		p3[0] = p[(i + 1) % PATCHNUM][0]; p3[1] = p[(i + 1) % PATCHNUM][1]; p3[2] = p[(i + 1) % PATCHNUM][2];
		pt1[0] = p2[0] - p1[0]; pt1[1] = p2[1] - p1[1]; pt1[2] = p2[2] - p1[2];
		pt2[0] = p3[0] - p1[0]; pt2[1] = p3[1] - p1[1]; pt2[2] = p3[2] - p1[2];
		CROSS(n, pt1, pt2);
		DGP_FLOAT rr = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		n[0] /= rr; n[1] /= rr; n[2] /= rr;
		nx += n[0]; ny += n[1]; nz += n[2];
	}
	nx /= (DGP_FLOAT)PATCHNUM; ny /= (DGP_FLOAT)PATCHNUM; nz /= (DGP_FLOAT)PATCHNUM;
	DGP_FLOAT rr = sqrt(nx * nx + ny * ny + nz * nz);
	nx /= rr; ny /= rr; nz /= rr;
}


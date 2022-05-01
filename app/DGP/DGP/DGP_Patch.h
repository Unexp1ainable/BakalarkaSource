#pragma once
#include "DGP_Defines.h"
#include "DGPObList.h"

class DGP_Vertex : public DGPObject
{
public:
	DGP_Vertex(const DGP_FLOAT xx, const DGP_FLOAT yy, const DGP_FLOAT zz) {
		pos[0] = xx; pos[1] = yy; pos[2] = zz;
		m_nIdentifiedPatchIndex = 0; patchList.RemoveAll();
	};
	virtual ~DGP_Vertex() {};

	DGP_VOID SetIndexNo(const DGP_INT index) { indexNo = index; };
	DGP_INT GetIndexNo() { return indexNo; };

	DGP_VOID SetIdentifiedIndex(DGP_INT index) { m_nIdentifiedPatchIndex = index; };
	DGP_INT GetIdentifiedIndex() { return m_nIdentifiedPatchIndex; };

	DGP_VOID SetPosition(const DGP_FLOAT xx, const DGP_FLOAT yy, const DGP_FLOAT zz) { pos[0] = xx; pos[1] = yy; pos[2] = zz; };
	DGP_VOID GetPosition(DGP_FLOAT& xx, DGP_FLOAT& yy, DGP_FLOAT& zz) { xx = pos[0]; yy = pos[1]; zz = pos[2]; };
	DGPObList& GetPatchList() { return patchList; };
private:
	DGP_INT indexNo, m_nIdentifiedPatchIndex;
	DGP_FLOAT pos[3];
	DGPObList patchList;
};

class DGP_Patch : public DGPObject
{
public:
	//DGP_Patch(const DGP_FLOAT xx, const DGP_FLOAT yy, const DGP_FLOAT zz) {centerPos[0]=xx;centerPos[1]=yy;centerPos[2]=zz;};
	DGP_Patch(const DGP_FLOAT nx, const DGP_FLOAT ny, const DGP_FLOAT nz) {
		targetNormal[0] = nx; 
		targetNormal[1] = ny; 
		targetNormal[2] = nz;
		for (DGP_INT i = 0; i < PATCHNUM; i++) {
			vertexArray[i] = NULL;
			excluded = false;
			visited = false;
			fixed = false;
			row = col = -1;
			m_nIdentifiedPatchIndex = 0;
		}
	};
	virtual ~DGP_Patch() {};

	DGP_VOID SetIndexNo(const DGP_INT _indexNo) { indexNo = _indexNo; };
	DGP_INT GetIndexNo() { return indexNo; };

	DGP_VOID SetVertex(const DGP_INT i, DGP_Vertex* vertex) { vertexArray[i] = vertex; };
	DGP_Vertex* GetVertex(const DGP_INT i) { return vertexArray[i]; };

	DGP_VOID SetExcludedFlag(const DGP_BOOL flag) { excluded = flag; };
	DGP_BOOL GetExcludedFlag() { return excluded; };

	DGP_VOID SetVisitedFlag(const DGP_BOOL flag) { visited = flag; };
	DGP_BOOL GetVisitedFlag() { return visited; };

	DGP_VOID SetFixedFlag(const DGP_BOOL flag) { fixed = flag; };
	DGP_BOOL GetFixedFlag() { return fixed; };

	DGP_VOID SetLocalPosition(const DGP_INT i, const DGP_FLOAT xx, const DGP_FLOAT yy, const DGP_FLOAT zz) { localPos[i][0] = xx; localPos[i][1] = yy; localPos[i][2] = zz; };
	DGP_VOID GetLocalPosition(const DGP_INT i, DGP_FLOAT& xx, DGP_FLOAT& yy, DGP_FLOAT& zz) { xx = localPos[i][0]; yy = localPos[i][1]; zz = localPos[i][2]; };

	DGP_VOID SetTargetNormal(const DGP_FLOAT nx, const DGP_FLOAT ny, const DGP_FLOAT nz) { targetNormal[0] = nx; targetNormal[1] = ny; targetNormal[2] = nz; };
	DGP_VOID GetTargetNormal(DGP_FLOAT& nx, DGP_FLOAT& ny, DGP_FLOAT& nz) { nx = targetNormal[0]; ny = targetNormal[1]; nz = targetNormal[2]; };

	DGP_VOID SetRowAndCol(const DGP_INT i, const DGP_INT j) { row = i; col = j; };
	DGP_VOID GetRowAndCol(DGP_INT& _row, DGP_INT& _col) { _row = row; _col = col; };

	DGP_VOID SetIdentifiedIndex(DGP_INT index) { m_nIdentifiedPatchIndex = index; };
	DGP_INT GetIdentifiedIndex() { return m_nIdentifiedPatchIndex; };
private:
	DGP_INT indexNo, row, col, m_nIdentifiedPatchIndex;
	DGP_BOOL excluded, visited, fixed;
	DGP_FLOAT targetNormal[3];
	DGP_FLOAT localPos[PATCHNUM][3];
	DGP_Vertex* vertexArray[PATCHNUM];
};

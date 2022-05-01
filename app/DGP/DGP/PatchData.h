#pragma once
#include "DGP_Defines.h"

class VertexData
{
public:
	VertexData() {for(int i=0;i<PATCHNUM;i++) patchArray[4]=NULL;};
	virtual ~VertexData();
	void GetPosition(DGP_FLOAT& xx, DGP_FLOAT& yy, DGP_FLOAT& zz) {xx=pos[0];yy=pos[1];zz=pos[2];};
	PatchData* GetPatch(const DGP_INT Id);
private:
	DGP_FLOAT pos[3];
	PatchData* patchArray[PATCHNUM]; 
};

class PatchData
{
public:
	PatchData(const DGP_FLOAT xx, const DGP_FLOAT yy, const DGP_FLOAT zz) {};
	virtual ~PatchData();
private:
	DGP_FLOAT center[3];
	DGP_FLOAT LocalPos[PATCHNUM][3];
	VertexData* vertexArray[PATCHNUM];
};



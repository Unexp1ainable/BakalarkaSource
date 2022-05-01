#pragma once
#include "DGP_Defines.h"
#include "DGPObList.h"
#include "DGP_Patch.h"

class DGP_FileIO
{
public:
	DGP_FileIO();
	virtual ~DGP_FileIO();

	DGP_BOOL ReadNormalData(DGP_CHAR* fileName, DGPObList* patchList, DGPObList* vertexList, DGP_INT& partNum);
	DGP_BOOL WriteResultData(DGP_CHAR* fileName, DGPObList* patchList, DGPObList* vertexList, DGP_BOOL withSurface = false);
	DGP_BOOL WriteDualData(DGP_CHAR* fileName, DGPObList* patchList, DGPObList* vertexList, DGP_BOOL withSurface = false);
private:
	DGP_VOID _reorderPatch(DGP_Vertex* vertex);
	DGP_INT _partitionModel(DGPObList* patchList);
};

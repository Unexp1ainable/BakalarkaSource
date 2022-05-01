#include <stdio.h>
#include <string.h>
#include "DGPHeap.h"
#include "DGP_FileIO.h"

DGP_FileIO::DGP_FileIO()
{
}

DGP_FileIO::~DGP_FileIO()
{

}

DGP_BOOL DGP_FileIO::ReadNormalData(DGP_CHAR* fileName, DGPObList* patchList, DGPObList* vertexList, DGP_INT& partNum)
{
	// check file validity
	DGP_CHAR exstr[4];
	DGP_INT length = (DGP_INT)(strlen(fileName));
	exstr[0] = fileName[length - 3];
	exstr[1] = fileName[length - 2];
	exstr[2] = fileName[length - 1];
	exstr[3] = '\0';
	if (_stricmp(exstr, "txt") != 0) return false;

	FILE* fp = fopen(fileName, "r");
	if (!fp) {
		printf("Can't read normal file!\n");
		return false;
	}

	DGP_Vertex** vertexFlag = NULL;
	// load height, width
	DGP_INT res_L = -1, res_W = -1;
	DGP_FLOAT temp1, temp2;
	fscanf(fp, "%lf %lf\n", &temp1, &temp2);
	res_L = (DGP_INT)temp1; res_W = (DGP_INT)temp2;
	if ((res_L <= 0) || (res_W <= 0)) return false;

	// create vertices pointers and initialize them to NULL
	vertexFlag = new DGP_Vertex * [(res_L + 1) * (res_W + 1)];
	for (DGP_INT i = 0; i < (res_L + 1) * (res_W + 1); i++) vertexFlag[i] = NULL;
	int count = 0;

	// load normals
	for (DGP_INT i = 0; i < res_W; i++) {
		for (DGP_INT j = 0; j < res_L; j++) {
			DGP_FLOAT nx = 0.0, ny = 0.0, nz = 0.0;
			fscanf(fp, "%lf %lf %lf\n", &nx, &ny, &nz);
			if ((nx == 0.0) && (ny == 0.0) && (nz == 0.0)) {
				continue;
			}
			count++;
			// create patch with given target normal
			DGP_Patch* patch = new DGP_Patch(nx, ny, nz);

			patch->SetRowAndCol(j, i); patchList->AddTail(patch);
			patch->SetIndexNo(patchList->GetCount());

			if (!vertexFlag[i * (res_L + 1) + j]) {
				DGP_Vertex* vertex = new DGP_Vertex(((DGP_FLOAT)j - 0.5), ((DGP_FLOAT)i - 0.5), 0.0);
				vertex->SetIndexNo(vertexList->GetCount());
				vertexList->AddTail(vertex);
				vertexFlag[i * (res_L + 1) + j] = vertex;
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(0, vertex);
			}
			else {
				DGP_Vertex* vertex = vertexFlag[i * (res_L + 1) + j];
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(0, vertex);
			}

			if (!vertexFlag[i * (res_L + 1) + j + 1]) {
				DGP_Vertex* vertex = new DGP_Vertex(((DGP_FLOAT)j + 0.5), (DGP_FLOAT)i - 0.5, 0.0);
				vertex->SetIndexNo(vertexList->GetCount());
				vertexList->AddTail(vertex);
				vertexFlag[i * (res_L + 1) + j + 1] = vertex;
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(1, vertex);
			}
			else {
				DGP_Vertex* vertex = vertexFlag[i * (res_L + 1) + j + 1];
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(1, vertex);
			}

			if (!vertexFlag[(i + 1) * (res_L + 1) + (j + 1)]) {
				DGP_Vertex* vertex = new DGP_Vertex(((DGP_FLOAT)j + 0.5), ((DGP_FLOAT)i + 0.5), 0.0);
				vertex->SetIndexNo(vertexList->GetCount());
				vertexList->AddTail(vertex);
				vertexFlag[(i + 1) * (res_L + 1) + (j + 1)] = vertex;
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(2, vertex);
			}
			else {
				DGP_Vertex* vertex = vertexFlag[(i + 1) * (res_L + 1) + (j + 1)];
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(2, vertex);
			}

			if (!vertexFlag[(i + 1) * (res_L + 1) + j]) {
				DGP_Vertex* vertex = new DGP_Vertex(((DGP_FLOAT)j - 0.5), ((DGP_FLOAT)i + 0.5), 0.0);
				vertex->SetIndexNo(vertexList->GetCount());
				vertexList->AddTail(vertex);
				vertexFlag[(i + 1) * (res_L + 1) + j] = vertex;
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(3, vertex);
			}
			else {
				DGP_Vertex* vertex = vertexFlag[(i + 1) * (res_L + 1) + j];
				vertex->GetPatchList().AddTail(patch);
				patch->SetVertex(3, vertex);
			}
		}
	}
	fclose(fp);
	delete[] vertexFlag;

	// partition the input model into different regions
	partNum = _partitionModel(patchList);
	return true;
}

DGP_BOOL DGP_FileIO::WriteDualData(DGP_CHAR* fileName, DGPObList* patchList, DGPObList* vertexList, DGP_BOOL withSurface)
{
	FILE* fp = fopen(fileName, "w");
	if (!fp) {
		printf("Can't write result file!\n");
		return false;
	}
	for (DGPPOSITION Pos = vertexList->GetHeadPosition(); Pos != NULL;) {
		DGP_Vertex* vertex = (DGP_Vertex*)vertexList->GetNext(Pos);
		fprintf(fp, "v ");
		DGP_FLOAT pp[3];
		vertex->GetPosition(pp[0], pp[1], pp[2]);
		fprintf(fp, "%lf %lf %lf\n", pp[0], pp[1], pp[2]);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
	for (DGPPOSITION Pos = patchList->GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(Pos);
		fprintf(fp, "f ");
		for (DGP_INT i = 0; i < PATCHNUM; i++) fprintf(fp, "%d ", patch->GetVertex(i)->GetIndexNo() + 1);
		fprintf(fp, "\n");
	}
	fclose(fp);
	return true;
}

DGP_BOOL DGP_FileIO::WriteResultData(DGP_CHAR* fileName, DGPObList* patchList, DGPObList* vertexList, bool withSurface)
{
	FILE* fp = fopen(fileName, "w");
	if (!fp) {
		printf("Can't write result file!\n");
		return false;
	}
	for (DGPPOSITION Pos = patchList->GetHeadPosition(); Pos != NULL;) {
		DGP_FLOAT pp[3], center[3] = { 0.0 };
		DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(Pos);
		for (DGP_INT i = 0; i < PATCHNUM; i++) {
			patch->GetVertex(i)->GetPosition(pp[0], pp[1], pp[2]);
			center[0] += pp[0]; center[1] += pp[1]; center[2] += pp[2];
		}
		center[0] /= (DGP_FLOAT)PATCHNUM;
		center[1] /= (DGP_FLOAT)PATCHNUM;
		center[2] /= (DGP_FLOAT)PATCHNUM;
		fprintf(fp, "v %lf %lf %lf\n", center[0], center[1], center[2]);
	}
	if (!withSurface) {
		fclose(fp);
		return true;
	}
	fprintf(fp, "\n");
	for (DGPPOSITION Pos = vertexList->GetHeadPosition(); Pos != NULL;) {
		DGP_Vertex* vertex = (DGP_Vertex*)vertexList->GetNext(Pos);
		if (vertex->GetPatchList().GetCount() < PATCHNUM) continue;
		_reorderPatch(vertex);
		fprintf(fp, "f ");
		for (DGPPOSITION Pos1 = vertex->GetPatchList().GetHeadPosition(); Pos1 != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos1);
			fprintf(fp, "%d ", patch->GetIndexNo());
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return true;
}

DGP_VOID DGP_FileIO::_reorderPatch(DGP_Vertex* vertex)
{
	DGP_Patch* patchArray[4];
	DGP_INT minRow, minCol, maxRow, maxCol, row, col;
	DGPPOSITION Pos = vertex->GetPatchList().GetHeadPosition();
	DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos);
	patch->GetRowAndCol(row, col);
	minRow = maxRow = row; minCol = maxCol = col;
	for (; Pos != NULL;) {
		patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos);
		patch->GetRowAndCol(row, col);
		if (row < minRow) minRow = row;
		if (row > maxRow) maxRow = row;
		if (col < minCol) minCol = col;
		if (col > maxCol) maxCol = col;
	}
	for (Pos = vertex->GetPatchList().GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)vertex->GetPatchList().GetNext(Pos);
		patch->GetRowAndCol(row, col);
		if ((row == minRow) && (col == minCol)) {
			patchArray[0] = patch;
		}
		else if ((row == maxRow) && (col == maxCol)) {
			patchArray[2] = patch;
		}
		else if (row == minRow) {
			patchArray[3] = patch;
		}
		else {
			patchArray[1] = patch;
		}
	}
	vertex->GetPatchList().RemoveAll();
	for (DGP_INT i = 0; i < 4; i++) vertex->GetPatchList().AddTail(patchArray[i]);
}

DGP_INT DGP_FileIO::_partitionModel(DGPObList* patchList)
{
	for (DGPPOSITION Pos = patchList->GetHeadPosition(); Pos != NULL;) {
		DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(Pos);
		patch->SetIdentifiedIndex(0);
	}
	int regionIndex = 1;
	do {
		DGP_Patch* seed = NULL;
		for (DGPPOSITION PosPatch = patchList->GetHeadPosition(); PosPatch != NULL;) {
			DGP_Patch* patch = (DGP_Patch*)patchList->GetNext(PosPatch);
			if (patch->GetIdentifiedIndex() == 0) { seed = patch; break; }
		}
		if (!seed) break;

		DGPObList list, keepList;
		list.AddTail(seed);
		seed->SetVisitedFlag(true);
		do {
			keepList.RemoveAll();
			for (DGPPOSITION Pos = list.GetHeadPosition(); Pos != NULL;) {
				DGP_Patch* patch = (DGP_Patch*)list.GetNext(Pos);
				patch->SetIdentifiedIndex(regionIndex);
				for (int i = 0; i < PATCHNUM; i++) {
					DGP_Vertex* vertex = patch->GetVertex(i);
					vertex->SetIdentifiedIndex(regionIndex);
				}
			}

			for (DGPPOSITION Pos = list.GetHeadPosition(); Pos != NULL;) {
				DGP_Patch* patch = (DGP_Patch*)list.GetNext(Pos);
				for (int i = 0; i < PATCHNUM; i++) {
					DGP_Vertex* vertex = patch->GetVertex(i);
					for (DGPPOSITION PosPatch = vertex->GetPatchList().GetHeadPosition(); PosPatch != NULL;) {
						DGP_Patch* patch1 = (DGP_Patch*)vertex->GetPatchList().GetNext(PosPatch);
						if ((patch1->GetIdentifiedIndex() == 0) && (!patch1->GetVisitedFlag())) {
							keepList.AddTail(patch1); patch1->SetVisitedFlag(true);
						}
					}
				}
			}
			list.RemoveAll(); list.AddTail(&keepList);
		} while (!(list.IsEmpty()));
		regionIndex++;
	} while (true);
	regionIndex--;
	return regionIndex;
}
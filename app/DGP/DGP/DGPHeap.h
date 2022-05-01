#pragma once
#include "DGP_Defines.h"
#include "DGPObList.h"

class DGPHeapNode : public DGPObject
{
public:
	DGPHeapNode() {index=0; attachedObj=NULL;};
	virtual ~DGPHeapNode() {};
	DGP_FLOAT GetValue() {return v;};
	DGP_VOID SetValue(DGP_FLOAT value) {v=value;};

	DGP_INT index;	//	this is the index for locating HeapNode in a heap
	DGP_VOID* attachedObj;
protected:
	DGP_FLOAT v;
};

class DGPHeap
{
public:
	DGPHeap(DGP_INT maxsize, DGP_BOOL minOrMax=true);	//	true - min Heap
												//	false - max Heap
	DGPHeap(DGPHeapNode** arr, DGP_INT n, DGP_BOOL minOrMax=true);
	virtual ~DGPHeap();
	
	const DGPHeapNode* operator[] (DGP_INT i);
	DGP_INT ListSize();
	DGP_BOOL ListEmpty();
	DGP_BOOL ListFull();

	DGP_VOID SetKetOnMinOrMax(DGP_BOOL flag);
	DGP_BOOL IsKeyOnMinOrMax();		//	true	- Keyed on min value
								//	false	- Keyed in max value
	
	DGP_BOOL Insert(DGPHeapNode* item);
	DGPHeapNode* RemoveTop();
	DGPHeapNode* GetTop();
	DGP_VOID AdjustPosition(DGPHeapNode* item);
	DGP_VOID Remove(DGPHeapNode* item);
	DGP_VOID ClearList();


	// utility functions for Delete/Insert to restore heap
	DGP_VOID FilterDown(DGP_INT i);
	DGP_VOID FilterUp(DGP_INT i);

private:
	DGP_BOOL bMinMax;	//	true	- Keyed on min value
					//	false	- Keyed in max value

	// hlist points at the array which can be allocated by the constructor (inArray == 0)
	//	or passed as a parameter (inArray == 1)
	DGPHeapNode** hlist;

	// amx elements allowed and current size of heap
	DGP_INT maxheapsize;
	DGP_INT heapsize;		// identifies end of list

	DGP_VOID Expand();
};

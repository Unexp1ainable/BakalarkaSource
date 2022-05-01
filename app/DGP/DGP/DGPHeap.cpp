#include "DGPHeap.h"

DGPHeap::DGPHeap(DGP_INT maxsize, DGP_BOOL minOrMax)
{
	hlist = new DGPHeapNode*[maxsize];
	maxheapsize = maxsize;
	heapsize = 0;
	bMinMax = minOrMax;
}

DGPHeap::DGPHeap(DGPHeapNode** arr, DGP_INT n, DGP_BOOL minOrMax)
{
	bMinMax = minOrMax;
	hlist = new DGPHeapNode*[n];
	maxheapsize = n;
	heapsize = n;
	for(DGP_INT i=0;i<n;i++) {hlist[i]=arr[i];hlist[i]->index=i;}

	DGP_INT currentpos = (heapsize-2)/2;
	while(currentpos>=0){
		FilterDown(currentpos);
		currentpos--;
	}
}

DGPHeap::~DGPHeap()
{
	ClearList();
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

const DGPHeapNode* DGPHeap::operator[] (DGP_INT i)
{
	return hlist[i];
}

DGP_INT DGPHeap::ListSize()
{
	return heapsize;
}

DGP_BOOL DGPHeap::ListEmpty()
{
	if(heapsize==0) return true;
	return false;
}

DGP_BOOL DGPHeap::ListFull()
{
	if(heapsize==maxheapsize) return true;
	return false;
}

DGP_VOID DGPHeap::Expand()
{
	DGPHeapNode** new_hlist = new DGPHeapNode*[maxheapsize*2];
	for(DGP_INT i=0;i<heapsize;i++) new_hlist[i] = hlist[i];
	delete [](DGPHeapNode**)hlist;
	hlist = new_hlist;
	maxheapsize = maxheapsize*2;
}

DGP_VOID DGPHeap::SetKetOnMinOrMax(DGP_BOOL flag)
{
	bMinMax = flag;
}

DGP_BOOL DGPHeap::IsKeyOnMinOrMax()
{
	return bMinMax;
}

DGP_VOID DGPHeap::AdjustPosition(DGPHeapNode* item)
{
	FilterUp(item->index);
	FilterDown(item->index);
}

DGP_VOID DGPHeap::Remove(DGPHeapNode* item)
{
	if(ListEmpty()) return;

	hlist[item->index]=hlist[heapsize-1];	
	hlist[item->index]->index=item->index;
	heapsize--;

	FilterDown(item->index);
}

DGP_BOOL DGPHeap::Insert(DGPHeapNode* item)
{
	if(ListFull()) Expand();
	hlist[heapsize] = item;
	item->index = heapsize;
	FilterUp(heapsize);
	heapsize++;
	return true;
}

DGPHeapNode* DGPHeap::GetTop() 
{
	return (hlist[0]);
}

DGPHeapNode* DGPHeap::RemoveTop()
{
	DGPHeapNode* tempitem;
	if(ListEmpty()) return 0;
	tempitem = hlist[0];
	hlist[0] = hlist[heapsize-1];	
	hlist[0]->index = 0;
	heapsize--;
	FilterDown(0);
	return tempitem;
}

DGP_VOID DGPHeap::ClearList()
{
	if(maxheapsize>0) delete hlist;
	maxheapsize=0;	heapsize=0;
}

DGP_VOID DGPHeap::FilterDown(DGP_INT i)
{
	DGP_INT currentpos, childpos;
	DGPHeapNode *target;

	currentpos = i;
	target = hlist[i];
	childpos = 2*i+1;

	while(childpos<heapsize){
		if(bMinMax){
			if((childpos+1<heapsize)&&
				((hlist[childpos+1]->GetValue())<=(hlist[childpos]->GetValue())))
				childpos=childpos+1;
			
			if((target->GetValue())<=(hlist[childpos]->GetValue()))
				break;
			else{
				hlist[currentpos]=hlist[childpos];
				hlist[currentpos]->index=currentpos;
				currentpos=childpos;
				childpos=2*currentpos+1;
			}
		}
		else{
			if((childpos+1<heapsize)&&
				((hlist[childpos+1]->GetValue())>=(hlist[childpos]->GetValue())))
				childpos=childpos+1;

			if((target->GetValue())>=(hlist[childpos]->GetValue()))
				break;
			else{
				hlist[currentpos]=hlist[childpos];
				hlist[currentpos]->index=currentpos;

				currentpos=childpos;
				childpos=2*currentpos+1;
			}
		}
	}
	hlist[currentpos] = target;	
	target->index = currentpos;
}

DGP_VOID DGPHeap::FilterUp(DGP_INT i)
{
	DGP_INT currentpos, parentpos;
	DGPHeapNode* target;

	currentpos = i;
	parentpos = (DGP_INT)((i-1)/2);
	target = hlist[i];

	while(currentpos!=0){
		if(bMinMax){
			if((hlist[parentpos]->GetValue())<=(target->GetValue()))
				break;
			else{
				hlist[currentpos] = hlist[parentpos];
				hlist[currentpos]->index = currentpos;
				currentpos = parentpos;
				parentpos = (DGP_INT)((currentpos-1)/2);
			}
		}
		else{
			if((hlist[parentpos]->GetValue())>=(target->GetValue()))
				break;
			else{
				hlist[currentpos] = hlist[parentpos];
				hlist[currentpos]->index = currentpos;
				currentpos = parentpos;
				parentpos = (DGP_INT)((currentpos-1)/2);
			}
		}
	}
	hlist[currentpos] = target;	
	target->index = currentpos;
}


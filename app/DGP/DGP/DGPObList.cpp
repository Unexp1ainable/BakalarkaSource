// DGPObList.cpp: implementation of the DGPObList class.
//
//////////////////////////////////////////////////////////////////////

#include "DGPObList.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DGPObList::DGPObList()
{
	headPos=NULL; tailPos=NULL;	nCount=0;
}

DGPObList::~DGPObList()
{
	RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

DGPPOSITION DGPObList::InsertBefore(DGPPOSITION rPosition, DGPObject* newElement)
{
	DGPPOSITION newPos;
	DGPObNode* newNode = new DGPObNode;	
	newNode->data = newElement;
	DGPObNode* currentNode = rPosition;
	if(currentNode){
		currentNode->InsertBefore(newNode);
		newPos=newNode;
	    nCount++;
		return newPos;
	}
	delete newNode;
	return NULL;
}

DGPPOSITION DGPObList::InsertAfter(DGPPOSITION rPosition, DGPObject* newElement)
{
	DGPPOSITION newPos;
	DGPObNode* newNode = new DGPObNode;	
	newNode->data = newElement;
	DGPObNode* currentNode = rPosition;
	if(currentNode){
		currentNode->InsertAfter(newNode);
		newPos=newNode;
		nCount++;
		return newPos;
	}
	delete newNode;
	return NULL;
}

DGPPOSITION DGPObList::AddHead( DGPObject* newElement )
{
	DGPObNode* newNode = new DGPObNode;	
	newNode->data = newElement;
	if(headPos) headPos->InsertBefore(newNode);
	headPos=newNode; nCount++;
	if(!tailPos) tailPos = newNode;
	return newNode;
}
	
DGPPOSITION DGPObList::AddTail( DGPObject* newElement )
{
	DGPObNode* newNode = new DGPObNode;	
	newNode->data = newElement;
	if(tailPos) tailPos->InsertAfter(newNode);
	tailPos=newNode; nCount++;
	if(!headPos) headPos = newNode;
	return newNode;
}

DGPObject* DGPObList::GetHead() {return headPos->data;}
DGPObject* DGPObList::GetTail() {return tailPos->data;}

DGPObject* DGPObList::RemoveHead()
{
	DGPObNode* tempNode=headPos;
	if(headPos){
		DGPObject* tempObj=tempNode->data;
		headPos = headPos->next;
		if(headPos) headPos->prev=NULL;
		nCount--;
		if(nCount==0) tailPos=NULL;
		delete tempNode;
		return tempObj;
	}
	return NULL;
}
	
DGPObject* DGPObList::RemoveTail()
{
	DGPObNode* tempNode = tailPos;
	if(tailPos){
		DGPObject* tempObj = tempNode->data;
		tailPos = tailPos->prev;
		if(tailPos) tailPos->next=NULL;
		nCount--;
		if(nCount==0) headPos = NULL;
		delete tempNode;
		return tempObj;
	}
	return NULL;
}

DGPObject* DGPObList::RemoveAt(DGPPOSITION rPosition)
{
	DGPObNode* prevNode = rPosition->prev;
	DGPObNode* nextNode = rPosition->next;

	if((nextNode==NULL)&&(prevNode==NULL)){
		DGPObject* tempObj = rPosition->data;
		RemoveAll();
		return tempObj;
	}

	if(!nextNode) return RemoveTail();
	if(!prevNode) return RemoveHead();

	DGPObject* tempObj = rPosition->data;
	prevNode->next = nextNode;
	nextNode->prev = prevNode;
	nCount--;

	if(nCount==0) {headPos=NULL; tailPos=NULL;}
	delete rPosition;
	return tempObj;
}

DGP_VOID  DGPObList::Remove(DGPObject* element)
{
	DGPPOSITION Pos;
	Pos=Find(element);
	if(Pos) RemoveAt(Pos);
}

DGP_VOID  DGPObList::RemoveAll()
{
	DGPObNode* node;
	for(node=headPos;node!=NULL;){
		DGPObNode* tempNode = node->next;
		delete node;
		node = tempNode;
	}
	nCount=0; headPos=NULL; tailPos=NULL;
}

DGPObject* DGPObList::GetNext(DGPPOSITION& rPosition)
{
	DGPPOSITION tempObj = rPosition;
	rPosition = tempObj->next;
	return tempObj->data;
}

DGPObject* DGPObList::GetAt(DGPPOSITION rPosition)
{
	return rPosition->data;
}
	
DGPObject* DGPObList::GetPrev(DGPPOSITION& rPosition)
{
	DGPPOSITION tempObj = rPosition;
	rPosition = tempObj->prev;
	return tempObj->data;
}

DGPPOSITION DGPObList::FindIndex(int index)
{
	DGP_INT n=0;
	for(DGPPOSITION Pos=GetHeadPosition();Pos!=NULL;n++) {
		if(n==index) return Pos;
		GetNext(Pos);
	}
	return NULL;
}

DGPPOSITION DGPObList::Find(DGPObject* element)
{
	DGP_INT n = 0;
	for(DGPPOSITION Pos=GetHeadPosition();Pos!=NULL;n++) {
		if(Pos->data==element) return Pos;
		GetNext(Pos);
	}
	return NULL;
}

DGP_VOID  DGPObList::AddHead(DGPObList* pNewList)
{
	for(DGPPOSITION Pos=pNewList->GetTailPosition();Pos!=NULL;)
		this->AddHead(pNewList->GetPrev(Pos));
}

DGP_VOID  DGPObList::AddTail( DGPObList* pNewList )
{
	for(DGPPOSITION Pos=pNewList->GetHeadPosition();Pos!=NULL;)
		this->AddTail(pNewList->GetNext(Pos));
}
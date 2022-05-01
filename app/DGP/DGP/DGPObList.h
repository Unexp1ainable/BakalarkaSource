// DGPObList.h: interface for the DGPObList class.
//
//////////////////////////////////////////////////////////////////////

#include "DGP_Defines.h"

#ifndef _DGPObject
#define _DGPObject
#define NULL	0
class DGPObject
{
public:
	DGPObject () {};
};
#endif

#ifndef _DGPObNode
#define _DGPObNode
class DGPObNode
{
public:
	DGPObNode(DGPObNode* ptrprev=NULL,DGPObNode* ptrnext=NULL)	{next=ptrnext; prev=ptrprev;};
	DGP_VOID InsertAfter(DGPObNode* p){
		DGPObNode*oldNextNode = next;
		next = p; p->prev = this;
		if(oldNextNode) {oldNextNode->prev=p; p->next=oldNextNode;}
	};
	DGPObNode* DeleteAfter(){
		DGPObNode* tempObj = next;
		if(next==NULL) return NULL;
		next = tempObj->next;
		next->prev = this;
		return tempObj;
	};

	DGP_VOID InsertBefore(DGPObNode* p){
		DGPObNode* oldPrevNode = prev;
		prev = p; p->next = this;
		if(oldPrevNode) {oldPrevNode->next=p;p->prev=oldPrevNode;}
	};
	DGPObNode* DeleteBefore(){
		DGPObNode* tempObj = prev;
		if(prev==NULL) return NULL;
		prev = tempObj->prev;
		prev->next = this;
		return tempObj;
	};
	DGPObNode* next;
	DGPObNode* prev;
	DGPObject* data;
};
typedef DGPObNode* DGPPOSITION;
#endif

#ifndef _DGPObList
#define _DGPObList
class DGPObList  
{
public:
	DGPObList();
	virtual ~DGPObList();

	DGPObject* GetHead();
	DGPObject* GetTail();
	DGPPOSITION GetHeadPosition() {return headPos;};
	DGPPOSITION GetTailPosition() {return tailPos;};
	DGPPOSITION FindIndex(DGP_INT index);
	DGPPOSITION Find(DGPObject* element);

	DGPPOSITION AddHead(DGPObject* newElement);
	DGP_VOID AddHead(DGPObList* pNewList);
	DGPPOSITION AddTail(DGPObject* newElement);
	DGP_VOID AddTail( DGPObList* pNewList );
	DGPObject* RemoveHead();
	DGPObject* RemoveTail();
	DGPObject* RemoveAt(DGPPOSITION rPosition);
	DGP_VOID Remove(DGPObject* element);
	DGP_VOID RemoveAll();

	DGPObject* GetNext(DGPPOSITION& rPosition);
	DGPObject* GetPrev(DGPPOSITION& rPosition);

	DGPObject* GetAt( DGPPOSITION rPosition );

	DGPPOSITION InsertBefore(DGPPOSITION rPosition, DGPObject* newElement);
	DGPPOSITION InsertAfter(DGPPOSITION rPosition, DGPObject* newElement);

	DGP_INT GetCount() {return nCount;};

	DGP_BOOL IsEmpty() {return ((nCount==0)?true:false);};

private:
	DGPPOSITION headPos;
	DGPPOSITION tailPos;

	DGP_INT nCount;
};
#endif
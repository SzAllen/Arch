

#ifndef __QUEUE_H__
#define __QUEUE_H__    

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

typedef struct _Queue
{
    int m_ReadPointer;
    int m_WritePointer;
	unsigned short	m_nElementSize;		//Element size

    unsigned char m_isUsed:1;
    unsigned char m_isEmpty:1;
    unsigned char m_isFull:1;
    unsigned char m_isWriteLock:1;
    unsigned char m_count:4;
	
	int	m_nBufferSize;
    uint8* m_pBuffer;
}Queue;

void* QUEUE_getNew(Queue* queue);
Bool QUEUE_addByte(Queue* queue, uint8 byte);
Bool QUEUE_add(Queue* queue, const void* element, int len);
Queue* QUEUE_create(int siz);

//Get the head element of queue
void* QUEUE_getHead(Queue* queue);

//Remove the head element;
void QUEUE_removeHead(Queue* queue);

//Get the head element and remove it
void* QUEUE_popGetHead(Queue* queue);

//Is queue empty 
//return: 1=Empty 0=Not Empty
Bool QUEUE_isEmpty(Queue* queue);
Bool QUEUE_isFull(Queue* queue);
void QUEUE_removeAll(Queue* queue);

Bool QUEUE_init(Queue* queue, void* pBuffer, unsigned short itemSize, unsigned short itemCount);
void QUEUE_reset(Queue* queue);
int QUEUE_GetCount(Queue* queue);

int QUEUE_getContinuousEleCount(Queue* queue);
int QUEUE_getElementCount(Queue* queue);
int QUEUE_PushOutEleArray(Queue* queue, void* pEleArrayBuf, int nMaxEleCount);

int QUEUE_getContinuousSpace(Queue* queue);
int QUEUE_getSpace(Queue* queue);
int QUEUE_PushInEleArray(Queue* queue, const void* pEleArray, int nEleCount);

#ifdef __cplusplus
}
#endif

#endif 


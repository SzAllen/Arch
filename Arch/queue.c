
#include "ArchDef.h"
#include "queue.h"

int QUEUE_getContinuousEleCount(Queue* queue)
{
	if(queue->m_isEmpty)
	{
		return 0;
	}
	else if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		return (queue->m_WritePointer - queue->m_ReadPointer) / queue->m_nElementSize;
	}
	else
	{
		return (queue->m_nBufferSize - queue->m_ReadPointer) / queue->m_nElementSize;
	}
}

int QUEUE_getElementCount(Queue* queue)
{
	if(queue->m_isEmpty)
	{
		return 0;
	}
	else if(queue->m_isFull)
	{
		return (queue->m_nBufferSize) / queue->m_nElementSize;
	}
	else if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		return (queue->m_WritePointer - queue->m_ReadPointer) / queue->m_nElementSize;
	}
	else
	{
		return (queue->m_nBufferSize - queue->m_ReadPointer + queue->m_WritePointer) / queue->m_nElementSize;
	}
}

//Return the element count of push out queue 
int QUEUE_PushOutEleArray(Queue* queue, void* pEleArrayBuf, int nMaxEleCount)
{
	int nLen = 0;
	int nCount = 0;
	int i = 0;
	uint8* pData = (uint8*)pEleArrayBuf;
	int nSpace = QUEUE_getContinuousEleCount(queue);

	while(nSpace && nMaxEleCount)
	{
		if(nSpace >= nMaxEleCount)
		{
			nLen = nMaxEleCount * queue->m_nElementSize;
			memcpy(&pData[i], &queue->m_pBuffer[queue->m_ReadPointer], nLen);
			nCount += nMaxEleCount;
			nMaxEleCount -= nSpace;
			
			queue->m_ReadPointer += nLen;
			if(queue->m_ReadPointer >= queue->m_nBufferSize)
			{
				queue->m_ReadPointer = 0;
			}
			queue->m_isFull = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isEmpty = True;
			}	
			break;
		}
		else
		{
			nLen = nSpace * queue->m_nElementSize;
			memcpy(&pData[i], &queue->m_pBuffer[queue->m_ReadPointer], nLen);
			i += nLen;
			nCount += nSpace;
			nMaxEleCount -= nSpace;
			
			queue->m_ReadPointer += nLen;
			if(queue->m_ReadPointer >= queue->m_nBufferSize)
			{
				queue->m_ReadPointer = 0;
			}
			queue->m_isFull = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isEmpty = True;
			}	
		}
		nSpace = QUEUE_getContinuousEleCount(queue);
	}
	
	return nCount;
}

int QUEUE_getContinuousSpace(Queue* queue)
{
	if(queue->m_isEmpty)
	{
		//queue->m_WritePointer = 0;
		//queue->m_ReadPointer = 0;
		return (queue->m_nBufferSize - queue->m_WritePointer) / queue->m_nElementSize;
	}
	else if(queue->m_isFull)
	{
		return 0;
	}
	else if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		return (queue->m_nBufferSize - queue->m_WritePointer) / queue->m_nElementSize;
	}
	else
	{
		return (queue->m_ReadPointer - queue->m_WritePointer) / queue->m_nElementSize;
	}
}

int QUEUE_getSpace(Queue* queue)
{
	if(queue->m_isFull)
	{
		return 0;
	}
	else if(queue->m_isEmpty)
	{
		return queue->m_nBufferSize/queue->m_nElementSize;
	}
	else if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		return (queue->m_nBufferSize - queue->m_WritePointer + queue->m_ReadPointer) / queue->m_nElementSize;
	}
	else
	{
		return (queue->m_ReadPointer - queue->m_WritePointer) / queue->m_nElementSize;
	}
}

//Return the element count of push in queue 
int QUEUE_PushInEleArray(Queue* queue, const void* pEleArray, int nEleCount)
{
	int nLen = 0;
	int nCount = 0;
	int i = 0;
	const uint8* pData = (uint8*)pEleArray;
	int nSpace = QUEUE_getContinuousSpace(queue);

	while(nSpace && nEleCount)
	{
		if(nSpace >= nEleCount)
		{
			nLen = nEleCount * queue->m_nElementSize;
			memcpy(&queue->m_pBuffer[queue->m_WritePointer], &pData[i], nLen);
			nCount += nEleCount;
			
			queue->m_WritePointer += nLen;
			if(queue->m_WritePointer >= queue->m_nBufferSize)
			{
				queue->m_WritePointer = 0;
			}
			queue->m_isEmpty = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isFull = True;
			}	
			break;
		}
		else
		{
			nLen = nSpace * queue->m_nElementSize;
			memcpy(&queue->m_pBuffer[queue->m_WritePointer], &pData[i], nLen);
			i += nLen;
			nCount += nSpace;
			nEleCount -= nSpace;
			
			queue->m_WritePointer += nLen;
			if(queue->m_WritePointer >= queue->m_nBufferSize)
			{
				queue->m_WritePointer = 0;
			}
			queue->m_isEmpty = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isFull = True;
			}
		}
		nSpace = QUEUE_getContinuousSpace(queue);
	}
	
	return nCount;
}

Bool QUEUE_add(Queue* queue, const void* element, int len)
{
	if(queue->m_isFull || queue->m_nElementSize < len) return False;

	memcpy(&queue->m_pBuffer[queue->m_WritePointer], element, len);

	queue->m_WritePointer += queue->m_nElementSize;
	if(queue->m_WritePointer >= queue->m_nBufferSize)
	{
		queue->m_WritePointer = 0;
	}
	
	queue->m_isEmpty = False;
	if(queue->m_WritePointer == queue->m_ReadPointer)
	{
		queue->m_isFull = True;
	}
	
	return True;
}

void* QUEUE_getHead(Queue* queue)
{
	if(queue->m_isEmpty)
	{
		return Null;
	}
	
	return &queue->m_pBuffer[queue->m_ReadPointer];
}

//Pop the head element;
void QUEUE_removeHead(Queue* queue)
{
	if(queue->m_isEmpty)
	{
		return ;
	}
	
	queue->m_ReadPointer += queue->m_nElementSize;
	if(queue->m_ReadPointer >= queue->m_nBufferSize)
	{
		queue->m_ReadPointer = 0;
	}
	
	queue->m_isFull = False;
	if(queue->m_ReadPointer == queue->m_WritePointer)
	{
		queue->m_isEmpty = True;
	}
}

void QUEUE_removeAll(Queue* queue)
{
	queue->m_ReadPointer = 0;
	queue->m_WritePointer = 0;
	queue->m_isEmpty = True;
	queue->m_isFull = False;
}

//Get the head element and pop it
void* QUEUE_popGetHead(Queue* queue)
{
	void* pvalue = QUEUE_getHead(queue);
	QUEUE_removeHead(queue);
	return pvalue;
}


//Is queue empty 
//return: 1=Empty 0=Not Empty
Bool QUEUE_isEmpty(Queue* queue)
{
	return queue->m_isEmpty;
}

Bool QUEUE_isFull(Queue* queue)
{
	return queue->m_isFull;
}

Bool QUEUE_init(Queue* queue, void* pBuffer, unsigned short itemSize, unsigned short itemCount)
{
	memset(queue, 0, sizeof(Queue));
	queue->m_isUsed = 1;

	queue->m_pBuffer = pBuffer;	
	queue->m_isEmpty = True;
	queue->m_nElementSize = itemSize;
	queue->m_nBufferSize = itemSize * itemCount;

	return True;
}

void QUEUE_reset(Queue* queue)
{
	queue->m_ReadPointer = 0;
	queue->m_WritePointer = 0;
	queue->m_isEmpty = True;
	queue->m_isFull = False;
	memset(queue->m_pBuffer, 0, queue->m_nBufferSize);
}

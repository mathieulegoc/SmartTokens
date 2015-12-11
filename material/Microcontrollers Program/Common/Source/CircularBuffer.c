/*
 * CircularBuffer.c
 *
 *  Created on: 30 mars 2015
 *      Author: Mathieu
 */

#include <CircularBuffer.h>
#include <dbg.h>
#include <dbg_uart.h>

extern uint64 timestamp;

bool isCBufferEmpty(CircularBuffer *cBuffer)
{
    if(cBuffer->validItems==0)
        return true;
    else
        return false;
}

bool putItemInCBuffer(CircularBuffer *cBuffer, AccelRawData* item)
{
	if(!item)
		return false;

    if(cBuffer->validItems>=BUFFER_LENGTH)
    	getItemFromCBuffer(cBuffer);

	memcpy(&(cBuffer->data[cBuffer->last]), item, sizeof(AccelRawData));
	if(cBuffer->validItems>0)
		cBuffer->last = (cBuffer->last+1)%BUFFER_LENGTH;

	cBuffer->validItems++;
	return true;
}

AccelRawData* getItemFromCBuffer(CircularBuffer *cBuffer)
{
	AccelRawData* item = NULL;
	DBG_vPrintf(true,"t%d f%d l%d n%d\n",(uint32)timestamp, cBuffer->first,cBuffer->last,cBuffer->validItems);
	if(!isCBufferEmpty(cBuffer))
    {
		DBG_vPrintf(true,"get\n");
        item = &(cBuffer->data[cBuffer->first]);
        cBuffer->first=(cBuffer->first+1)%BUFFER_LENGTH;
        cBuffer->validItems--;
    }
    return item;
}

/*
 * CircularBuffer.h
 *
 *  Created on: 30 mars 2015
 *      Author: Mathieu
 */

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <stdlib.h>
#include <jendefs.h>
#include <string.h>
#include "config.h"
#include <stdlib.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define BUFFER_LENGTH   30


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct circularBuffer{
	uint8     		first;
    uint8    		last;
    uint8     		validItems;
    AccelRawData    data[BUFFER_LENGTH];
} CircularBuffer;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
bool isCBufferEmpty(CircularBuffer *cBuffer);
bool putItemInCBuffer(CircularBuffer *cBuffer, AccelRawData* item);
AccelRawData* getItemFromCBuffer(CircularBuffer *cBuffer);

#endif /* CIRCULARBUFFER_H_ */

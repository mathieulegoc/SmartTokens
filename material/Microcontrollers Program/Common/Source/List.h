/*
 * List.h
 *
 *  Created on: Mar 28, 2015
 *      Author: Mathieu
 */

#ifndef LIST_H_
#define LIST_H_
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


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct ListElement ListElement;
struct ListElement{
    AccelRawData* value;
    struct ListElement* nxt;
};

typedef ListElement* List;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
List addAsFirst(List list, AccelRawData* value);
List addAsLast(List list, AccelRawData* value);
List deleteFirst(List list);
List deleteLast(List list);
int count(List list);
List deleteList(List list);
AccelRawData* get(List list, int index);
AccelRawData* getFirst(List list);
AccelRawData* getLast(List list);

#endif /* LIST_H_ */

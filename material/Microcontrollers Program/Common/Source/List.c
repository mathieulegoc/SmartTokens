/*
 * List.c
 *
 *  Created on: Mar 28, 2015
 *      Author: Mathieu
 */
#include <List.h>


List addAsFirst(List list, AccelRawData *value)
{
    ListElement* newElement = malloc(sizeof(ListElement));
	if(value!=NULL)
		memcpy(&(newElement->value),value,sizeof(value));

    newElement->nxt = list;
    return newElement;
}

List addAsLast(List list, AccelRawData *value)
{
	ListElement* newElement = malloc(sizeof(ListElement));
	if(value!=NULL)
		memcpy(&(newElement->value),value,sizeof(value));

    newElement->nxt = NULL;

    if(list == NULL)
        return newElement;
    else{
        ListElement* temp = list;
        while(temp->nxt != NULL)
            temp = temp->nxt;

        temp->nxt = newElement;
        return list;
    }
}

List deleteFirst(List list)
{
    if(list != NULL){
        ListElement* newFirst = list->nxt;
        free(list);
        return newFirst;
    }
    else
        return NULL;
}

List deleteLast(List list)
{
    if(list == NULL)
        return NULL;

    if(list->nxt == NULL){
        free(list);
        return NULL;
    }

    ListElement* tmp = list;
    ListElement* ptmp = list;

    while(tmp->nxt != NULL){
        ptmp = tmp;
        tmp = (List)tmp->nxt;
    }

    ptmp->nxt = NULL;
    free(tmp);
    return list;
}

int count(List list)
{
    if(list == NULL)
        return 0;

    return count(list->nxt)+1;
}

List deleteList(List list)
{
	if(list == NULL)
		return NULL;
	else{
		ListElement *tmp;
		tmp = list->nxt;
		free(list);
		return deleteList(tmp);
	}
}

AccelRawData* get(List list, int index)
{
	if(list == NULL)
		return NULL;
	else if(index == 0)
		return &(list->value);
	return get(list->nxt,index-1);
}

AccelRawData* getFirst(List list)
{
	if(list == NULL)
		return NULL;

	return &(list->value);
}

AccelRawData* getLast(List list)
{
	if(list == NULL)
		return NULL;
	return get(list, count(list)-1);
}

/*
 * list.c
 *
 *  Created on: 13. stu 2015.
 *      Author: Marko
 */

#include "ets_sys.h"
#include "mem.h"
#include "list.h"

list_t *list_createList(){
    list_t *result = (list_t *)os_zalloc(sizeof(list_t));
    return result;
}

void list_addItem(list_t *list, void *item){
    if(list->item == NULL){
        list->item = item;
        return;
    }

    list_t *current = list;
    while(current->next != NULL){
        current = current->next;
    }
    current->next = list_createList();
    current->next->prev = current;
    current->next->item = item;
}

int16_t list_getLength(list_t *list){
    list_t *temp = list;
    int16_t counter = 0;
    while(temp != NULL){
        temp = temp->next;
        counter++;
    }
    return counter;
}

void *list_toArray(list_t *list){
    void **array = (void **) os_zalloc(sizeof(void *) * list_getLength(list));

    list_t *temp = list;
    int i = 0;
    while(temp != NULL){
        array[i++] = (void *) temp->item;
        temp = temp->next;
    }

    return array;
}

list_t *list_removeItem(list_t *list, void *item, bool releaseItem){
    list_t *current = list;

    while(current != NULL){
        if(current->item == item){
            list_t *prev = current->prev;
            list_t *next = current->next;

            if(releaseItem)
                os_free(current->item);

            if(next != NULL)
                next->prev = prev;

            if(current->prev != NULL)
                prev->next = next;

            os_free(current);
            if(current == list){
                if(prev != NULL){
                    return prev;
                }

                if(next != NULL){
                    return next;
                }

                return list_createList();
            }

            return list;
        }
        current = current->next;
    }
    return list;
}

void list_deleteList(list_t *list, bool releaseItems){
    if(list->next != NULL){
        list_deleteList(list->next, releaseItems);
    }

    if(releaseItems)
        os_free(list->item);
    os_free(list);
}

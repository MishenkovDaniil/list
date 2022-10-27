#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef int list_elem_t;

static const int INITIAL = 0;
static const int NULL_ELEM = 0;
static const int EMPTY = -1;
static const int POISON = 0xDEADBEEF;
static int ERROR = 0;

enum errors
{
    LIST_INSERT_ERROR        = 0x1 << 0,
    LIST_INCORREST_PUT_PLACE = 0x1 << 1,
    LIST_INCORRECT_CAPACITY  = 0x1 << 2,
    LIST_INCORRECT_POP_PLACE = 0x1 << 3
};

struct List_elem
{
    list_elem_t data = 0;
    int next = 0;
    int prev = 0;
};

struct List
{
    int head = 0;
    int tale = 0;
    int size = 0;
    int free = 0;
    int capacity = 0;

    List_elem *elems = nullptr;
};


void list_ctor (List *list, int capacity, int *err = &ERROR);
void fill_list (List *list, int start, int *err);
void list_dtor (List *list, int *err = &ERROR);
int list_insert (List *list, int put_place, list_elem_t value, int *err = &ERROR);
void list_error (List *list, int *err);
list_elem_t list_pop (List *list, int pop_place, int *err = &ERROR);

void list_ctor (List *list, int capacity, int *err)
{
    assert (list);

    if (capacity <= 0)
    {
        *err |= LIST_INCORRECT_CAPACITY;
        list_error (list, err);
    }

    list->elems = (List_elem *)calloc (capacity, sizeof (List_elem)); //check if nullptr
    assert (list->elems);

    list->capacity = capacity;

    list->head = list->tale = list->size = INITIAL;

    list->elems[NULL_ELEM].data = list->elems[NULL_ELEM].prev = list->elems[NULL_ELEM].next = INITIAL;

    list->free = 1;

    fill_list (list, INITIAL + 1, err);
}

int list_insert (List *list, int put_place, list_elem_t value, int *err)
{
    assert (list);
    assert (err);
/*
    if (list->size == list->capacity)
    {
        list_realloc (list);
    }*/


    int insert_index = list->free;
    printf ("insert_index %d", insert_index);

    if (put_place < 0)
    {
        *err |= LIST_INCORREST_PUT_PLACE;
        list_error (list, err);

        return POISON;
    }
    else if (!(list->head))
    {
        list->head = list->tale = put_place + 1;
        list->elems[list->head].prev = NULL_ELEM;
        list->elems[list->head].data = value; //check
    }
    else if (!(put_place))
    {

        list->elems[insert_index].data = value;
        list->elems[insert_index].next = list->head;

        list->elems[list->head].prev = insert_index;

        list->head = insert_index;
    }
    else if (list->elems[put_place].data == POISON)
    {
        *err |= LIST_INSERT_ERROR;

        list_error (list, err);

        return POISON;
    }
    else if (put_place == list->tale)
    {
        list->elems[insert_index].data = value;
        list->elems[insert_index].prev = list->tale;

        list->tale = insert_index;
    }
    else
    {
        list->elems[insert_index].data = value;
        list->elems[insert_index].prev = put_place;
        list->elems[insert_index].next = list->elems[put_place].next;

        list->elems[put_place].next = insert_index;
    }

    list->free = list->elems[list->free].next;//check if 0
    (list->size)++;

    return insert_index;
}

list_elem_t list_pop (List *list, int pop_place, int *err)
{
    if (list->elems[pop_place].data == POISON)
    {
        fprintf (stderr, "POP_ERROR: incorrect pop place");

        *err |= LIST_INCORRECT_POP_PLACE;
        list_error (list, err);

        return POISON;
    }

    int return_value = list->elems[pop_place].data;

    if (pop_place == list->head)
    {
        list->head = list->elems[pop_place].next;
        list->elems[list->head].prev = NULL_ELEM;
    }
    else if (pop_place == list->tale)
    {
        list->tale = list->elems[pop_place].prev;
        list->elems[list->tale].next = NULL_ELEM;
    }
    else
    {
        list->elems[list->elems[pop_place].prev].next = list->elems[pop_place].next;
        list->elems[list->elems[pop_place].next].prev = list->elems[pop_place].prev;
    }

    list->elems[pop_place].data = POISON;
    list->elems[pop_place].next = list->free;
    list->elems[pop_place].prev = EMPTY;

    list->free = pop_place;

    (list->size)--;

    return return_value;
}

void list_dtor (List *list, int *err)
{
    if (list != nullptr && list->elems != nullptr)
    {
        free (list->elems);

        list->elems = nullptr;
        list = nullptr;
    }
}

int list_error (List *list, int *err)
{
    do
    {
        if (list)
    }while(0);

}

void list_error (List *list, int *err)
{
    printf ("ERROR");
}

void fill_list (List *list, int start, int *err)
{
    while (start < list->capacity)
    {
        list->elems[start].data = POISON;
        list->elems[start].prev = EMPTY;

        if (start == list->capacity - 1)
        {
            list->elems[start].next = NULL_ELEM;
        }
        else
        {
            list->elems[start].next = start + 1;
        }

        start++;
    }
}

#endif /* LIST_H */

#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef int elem_t;

static const int START = 1;
static const int STATIC_ELEM = -1;
static const int POISON = 0xDEADBEEF:
static const int ERRNO = 0;

enum errors
{
    LIST_INSERT_ERROR        = 0x1 << 0,
    LIST_INCORREST_PUT_PLACE = 0x1 << 1,
    LIST_INCORRECT_CAPACITY  = 0x1 << 2,
    LIST_INCORRECT_POP_PLACE = 0x1 << 3
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

struct List_elem
{
    elem_t data = 0;
    int next = 0;
    int prev = 0;
};

void list_ctor (List *list, int capacity, int *err = &ERRNO);
void fill_list (List *list, int *err);
void list_dtor (List *list, int *err = &ERRNO);
void list_insert (List *list, int put_place, elem_t value, int *err = &ERRNO);
void list_error (List *list, int *err);
elem_t list_pop (List *list, int pop_place, int *err);

void list_ctor (List *list, int capacity)
{
    if (capacity <= 0)
    {
        *err |= LIST_INCORRECT_CAPACITY;
        list_error (list, err);
    }
    list->elems = (List_elem *)calloc (capacity, sizeof (List_elem)); //check if nullptr

    fill_list (list);
}

void list_insert (List *list, int put_place, elem_t value, int *err)
{
    int index = find_space(list);

    if (put_place < 0)
    {
        *err |= LIST_INCORREST_PUT_PLACE;
        list_error (list, err);

        break;
    }
    if (!(list->head))
    {
        list->head = list->tale = put_place + 1;
        list->elems[list->head].prev = 0;
        list->elems[list->head].data = value; //check
    }
    else if (!(put_place))
    {
        list->elems[index].data = value;
        list->elems[index].next = list->head;

        list->elems[list->head].prev = index;

        list->head = index;
    }
    else if (list->elems[put_place] == POISON)
    {
        *err |= INSERT_ERROR;

        list_error (list, err);

        break;
    }
    else if (put_place == list->tale)
    {
        list->elems[index].data = value;
        list->elems[index].prev = list->tale;

        list->tale = index;
    }
    else
    {
        list->elems[index].data = value;
        list->elems[index].prev = put_place;
        list->elems[index].next = list->elems[put_place].next;

        list->elems[put_place].next = index;
    }
}

elem_t list_pop (List *list, int pop_place, int *err)
{
    if (list->elems[put_place].data = POISON)
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
        list->elems[list->head].prev = 0;
    }
    else if (pop_place == list->tale)
    {
        list->tale = list->elems[pop_place].prev;
    }
    else
    {
        list->elems[list->elems[pop_place].prev].next = list->elems[pop_place].next;
        list->elems[list->elems[pop_place].next].prev = list->elems[pop_place].prev;
    }

    list->elems[pop_place].data = POISON;
    list->elems[pop_place].next = STATIC_ELEM;
    list->elems[pop_place].prev = STATIC_ELEM;
}

void list_dtor (List *list)
{
    if (list != nullptr && list->elems != nullptr)
    {
        free (list->elems);

        list->elems = nullptr;
        list = nullptr;
    }
}

void fill_list(List *list)
{
    int value = START;

    list->elems[0].next = 0;
    list->elems[0].prev = 0;
    list->elems[0].data = POISON;

    while (value < LIST_SIZE)
    {
        list->elems[value].data = POISON;
        list->elems[value].next = STATIC_ELEM;
        list->elems[value].prev = STATIC_ELEM;

        if (value == LIST_SIZE - 1)
        {
            list->elems[value].next = 0;
        }
        if (value == START)
        {
            list->elems[value].prev = 0;
        }

        value++;
    }
}


#endif /* LIST_H */

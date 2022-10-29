#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../ptr_check.h"
#undef ERROR
static int ERROR = 0;

#define CHECK(case, action) \
if (case)                   \
{                           \
    action                  \
}

typedef int list_elem_t;
typedef unsigned long long canary_t;


static const int INITIAL = 0;
static const int NULL_ELEM = 0;
static const int EMPTY = -1;
static const int POISON = 0xDEADBEEF;
static const canary_t CANARY = 0xAB8EACAAAB8EACAA;

static FILE *list_log = fopen ("list_log.txt", "w");
static FILE *list_graph = fopen ("list_graph.txt", "w");

enum errors
{
    LIST_FOPEN_FAIL             = 0x1 << 0,
    LIST_ALLOCATION_FAIL        = 0x1 << 1,
    LIST_BAD_READ_LIST          = 0x1 << 2,
    LIST_BAD_READ_DATA          = 0x1 << 3,
    LIST_INCORRECT_SIZE         = 0x1 << 4,
    LIST_INCORRECT_CAPACITY     = 0x1 << 5,
    LIST_INSERT_ERROR           = 0x1 << 6,
    LIST_INCORRECT_INSERT_PLACE = 0x1 << 7,
    LIST_INCORRECT_POP_PLACE    = 0x1 << 8,
    LIST_POP_FROM_EMPTY_LIST    = 0x1 << 9,
    LIST_PREV_NEXT_OP_ERR       = 0x1 << 10,
    LIST_FREE_ELEM_NOT_EMPTY    = 0x1 << 11,
    LIST_VIOLATED_LIST          = 0x1 << 12
};

struct List_elem
{
    list_elem_t data = 0;
    int next = 0;
    int prev = 0;
};

struct List
{
    #ifdef CANARY_PROT
    canary_t left_canary = CANARY;
    #endif

    int head = 0;
    int tale = 0;
    int size = 0;
    int free = 0;
    int capacity = 0;

    List_elem *elems = nullptr;

    #ifdef CANARY_PROT
    canary_t right_canary = CANARY;
    #endif
};


void list_ctor (List *list, int capacity, int *err = &ERROR);
void fill_list (List *list, int start, int *err);
void list_dtor (List *list, int *err = &ERROR);
int list_insert (List *list, int put_place, list_elem_t value, int *err = &ERROR);
int list_error (List *list, int *err);
list_elem_t list_pop (List *list, int pop_place, int *err = &ERROR);
void list_dump (List *list, int *err);
void dump_list_members (List *list, int *err);
void dump_elems (List *list, int *err);
void dump_list_errors (List *list, int *err);
void make_graph (List *list);
void list_realloc (List *list, int linearize = false, int *err = &ERROR);
void linearize_list (List *list, int *err);


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

    list_error (list, err);
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

    list_error (list, err);

    int insert_index = list->free;

    int previous_free = list->free;
    list->free = list->elems[previous_free].next;//check if 0

    if (put_place < 0)
    {
        *err |= LIST_INCORRECT_INSERT_PLACE;
        list_error (list, err);

        list->free = previous_free;

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
        list->elems[insert_index].prev = NULL_ELEM;

        list->elems[list->head].prev = insert_index;

        list->head = insert_index;
    }
    else if (list->elems[put_place].data == POISON)
    {
        *err |= LIST_INSERT_ERROR;

        list_error (list, err);

        list->free = previous_free;

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

    (list->size)++;

    list_error (list, err);

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
    if (list->size == 0)
    {
        fprintf (stderr, "POP_ERROR: pop from empty list");

        *err |= LIST_POP_FROM_EMPTY_LIST;
        list_error (list, err);

        return POISON;
    }

    list_error (list, err);

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

    list_error (list, err);

    return return_value;
}

void list_realloc (List *list, int linearize, int *err)
{
    if (list->size >= list->capacity - 1)
    {
        list->capacity *= 2;
        if (linearize)
        {
            linearize_list (list, err);
        }
        list->elems = (List_elem *)realloc (list->elems, list->capacity * sizeof (List_elem));
    }
}

void linearize_list (List *list, int *err)
{
    List_elem *temp_elems = (List_elem *)calloc (list->capacity, sizeof (List_elem));

    int phys_index  = list->head;
    int logic_index = 1;

    while (logic_index <= list->size)
    {
        temp_elems[logic_index].data = list->elems[phys_index].data;
        temp_elems[logic_index].next = logic_index + 1;
        temp_elems[logic_index].prev = logic_index - 1;

        phys_index = list->elems[phys_index].next;
        logic_index++;
    }
    while (logic_index < list->capacity)
    {
        temp_elems[logic_index].data = POISON;
        temp_elems[logic_index].next = logic_index + 1;
        temp_elems[logic_index].prev = EMPTY;

        logic_index++;
    }

    temp_elems[--logic_index].next = NULL_ELEM;

    list->head = 1;
    list->tale = list->size;

    list->elems = temp_elems;
}

void list_dtor (List *list, int *err)
{
    if (list != nullptr && list->elems != nullptr)
    {
        free (list->elems);

        list->elems = nullptr;
        list = nullptr;
    }
    else
    {
        printf ("\nnothing to dtor\n");
    }
}

int list_error (List *list, int *err)
{
    int index = list->head;
    int counter = 0;

    do
    {
        CHECK (is_bad_read_ptr (list), {*err |= LIST_BAD_READ_LIST;
                                 break;})
        CHECK (is_bad_read_ptr (list->elems), {*err |= LIST_BAD_READ_DATA;
                                 break;})
        CHECK (list->size < 0, *err |= LIST_INCORRECT_SIZE;)
        CHECK (list->capacity <= 0, *err |= LIST_INCORRECT_CAPACITY;)

        if (!(*err & LIST_INCORRECT_SIZE) && !(*err & LIST_INCORRECT_CAPACITY))
        {
            while (counter++ < list->size - 1)
            {
                CHECK (list->elems[list->elems[index].next].prev != index, *err |= LIST_PREV_NEXT_OP_ERR;)
                index = list->elems[index].next;
            }

            counter = 0;
            index = list->free;

            while (counter++ < list->capacity - list->size - 1)
            {
                CHECK (list->elems[index].prev != EMPTY, *err |= LIST_FREE_ELEM_NOT_EMPTY;)
            }
        }

        #ifdef CANARY_PROT
        CHECK (list->left_canary != CANARY || list->right_canary != CANARY, *err |= LIST_VIOLATED_LIST;)
        #endif
    }while(0);

    if (*err)
    {
        list_dump (list, err);
    }
}

void list_dump (List *list, int *err)
{
    fprintf (list_log, "list\n");

    dump_list_members (list, err);
    dump_list_errors  (list, err);

    fprintf (list_log, "\n\n\n\n\n");

    make_graph (list);
}

void make_graph (List *list)
{
    int idx = 0;
    fprintf (list_graph, "digraph {\n\trankdir = LR;\n\t");
    while (idx < list->capacity)
    {
        fprintf (list_graph, "\tlabel_%d [shape = record, label = \"idx[%d]\\n | data [%d]\\n | next [%d]\\n | prev [%d]\"];\n ",
                                 idx, idx, list->elems[idx].data, list->elems[idx].next, list->elems[idx].prev);
        idx++;
    }

    idx = 0;
    fprintf (list_graph, "\t{edge [style = \"invis\", arrowsize = 44, weight = 1000];\n\t");
    while (idx < list->capacity - 1)
    {
        fprintf (list_graph, "label_%d->", idx);
        idx++;
    }
    fprintf (list_graph, "label_%d;\n\t}\n", idx);

    fprintf (list_graph, "\t{edge [color = \"purple\", arrowsize = 5, weight = 1];\n\t");
    int counter = 0;
    idx = list->head;

    while (counter++ < list->size)
    {
        fprintf (list_graph, "\tlabel_%d->label_%d [fillcolor = \"lightgrey\"];\n", idx, list->elems[idx].next);
        idx = list->elems[idx].next;
    }
    fprintf (list_graph, "}");
}

void dump_list_members (List *list, int *err)
{
    fprintf (list_log, "list head is %d\nlist tale is %d\n"
                       "list size id %d\nlist capacity id %d\n"
                       "list free is %d\nlist elems is [%p]\n",
                       list->head, list->tale, list->size, list->capacity, list->free, list->elems);
    dump_elems (list, err);
}

void dump_elems (List *list, int *err)
{
    for (int index = 0; index < list->capacity; index++)
    {
        fprintf (list_log, "idx[%d]\t data [%d]\t next is [%d]\t prev is [%d]\n", index, list->elems[index].data, list->elems[index].next, list->elems[index].prev);
    }
}

void dump_list_errors (List *list, int *err)
{
    const char *status[10] = {};

    do
    {
        if (*err & LIST_FOPEN_FAIL)
        {
            fprintf (stderr, "opening of log file failed");
            break;
        }
        if (*err & LIST_ALLOCATION_FAIL)
        {
            fprintf (list_log, "calloc failed");
        }
        if (*err & LIST_BAD_READ_LIST)
        {
            fprintf (list_log, "list is a bad ptr");
        }
        if (*err & LIST_BAD_READ_DATA)
        {
            fprintf (list_log, "list elems is a bad ptr");
        }
        if (*err & LIST_INCORRECT_CAPACITY)
        {
            fprintf (list_log, "capacity is incorrect (<=0)");
        }
        if (*err & LIST_PREV_NEXT_OP_ERR)
        {
            fprintf (list_log, "next element of previous is not equal to original");
        }
        if (*err & LIST_FREE_ELEM_NOT_EMPTY)
        {
            fprintf (list_log, "free element is not empty");
        }
        if (*err & LIST_VIOLATED_LIST)
        {
            fprintf (list_log, "access rights of stack are invaded");
        }
    }while (0);
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

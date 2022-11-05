#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../ptr_check.h"

// Fix
static unsigned int ERRNO = 0;

typedef int list_elem_t;
typedef unsigned long long canary_t;

static const int INITIAL = 0;
static const int NULL_ELEM = 0;
static const int EMPTY = -1;
static const int POISON = 0xDEADBEEF;
static const canary_t CANARY = 0xAB8EACAAAB8EACAA;
static const int CANARY_NUM = 2;
static const int MAX_CAPACITY = 100000;

static FILE *list_log = fopen ("list_log.html", "w");

enum errors
{
    LOG_FOPEN_FAIL             = 0x1 << 0,
    LIST_ALLOCATION_FAIL        = 0x1 << 1,
    LIST_BAD_READ_LIST          = 0x1 << 2,
    LIST_BAD_READ_DATA          = 0x1 << 3,
    LIST_INCORRECT_SIZE         = 0x1 << 4,
    LIST_INCORRECT_CAPACITY     = 0x1 << 5,
    LIST_INSERT_ERROR           = 0x1 << 6,
    LIST_INCORRECT_INSERT_PLACE = 0x1 << 7,
    LIST_INCORRECT_REMOVE_PLACE    = 0x1 << 8,
    LIST_REMOVE_FROM_EMPTY_LIST    = 0x1 << 9,
    LIST_PREV_NEXT_OP_ERR       = 0x1 << 10,
    LIST_FREE_ELEM_NOT_EMPTY    = 0x1 << 11,
    LIST_VIOLATED_LIST          = 0x1 << 12,
    LIST_VIOLATED_DATA          = 0x1 << 13,
};

struct List_elem
{
    list_elem_t data = POISON;
    List_elem *next = nullptr;
    List_elem *prev = nullptr;
};

struct List
{
    #ifdef CANARY_PROT
    canary_t left_canary = CANARY;
    #endif

    int size = 0;

    List_elem *zero_elem = nullptr;

    #ifdef CANARY_PROT
    canary_t right_canary = CANARY;
    #endif
};


void list_ctor          (List *list,                                          unsigned int *err = &ERRNO);
void list_dtor          (List *list,                                          unsigned int *err = &ERRNO);
List_elem *list_insert  (List *list, List_elem *put_place, list_elem_t value, unsigned int *err = &ERRNO);
list_elem_t list_remove (List *list, List_elem *remove_place,                 unsigned int *err = &ERRNO);
List_elem *find_elem    (List *list, int phys_index, unsigned int *err = &ERRNO);
void list_free          (List *list);
void set_error_bit      (unsigned int *error, int bit);

int check_list          (List *list, unsigned int *err);
void list_dump          (List *list, unsigned int *err);
void dump_list_members  (List *list, unsigned int *err);
void dump_elems         (List *list, unsigned int *err);
void dump_list_errors   (List *list, unsigned int *err);
void make_graph         (List *list, FILE *list_graph);

void set_error_bit (unsigned int *error, int bit)
{
    *error |= bit;
}

void list_ctor (List *list, unsigned int *err)
{
    assert (list);

    if ( is_bad_ptr (list))
    {
        set_error_bit (err, LIST_BAD_READ_LIST);
    }
    else
    {
        list->zero_elem = (List_elem *)calloc(1, sizeof (List_elem));

        list->zero_elem->data = POISON;
        list->zero_elem->prev = list->zero_elem;
        list->zero_elem->next = list->zero_elem;

        list->size = INITIAL;
    }

    check_list (list, err);
}

List_elem *list_insert (List *list, List_elem *put_place, list_elem_t value, unsigned int *err)
{
    assert (list);
    assert (err);

    check_list (list, err);

    List_elem *temp_elem = (List_elem *)calloc (1, sizeof (List_elem));

    /*if (put_place > list->size)
    {
        set_error_bit (err, LIST_INCORRECT_INSERT_PLACE);
        check_list (list, err);

        //return POISON;
    }*/
    /*else if (list->elems[put_place].prev == EMPTY && put_place != NULL_ELEM)
    {
        set_error_bit (err, LIST_INSERT_ERROR);

        check_list (list, err);

        return POISON;
    }*/

    temp_elem->data = value;
    temp_elem->prev = put_place;
    temp_elem->next = put_place->next;

    put_place->next = temp_elem;
    temp_elem->next->prev = temp_elem;

    (list->size)++;

    check_list (list, err);

    return temp_elem;
}

list_elem_t list_remove (List *list, List_elem *remove_elem, unsigned int *err)
{
    if (remove_elem == nullptr)
    {
        fprintf (stderr, "REMOVE_ERROR: incorrect remove place");

        set_error_bit (err, LIST_INCORRECT_REMOVE_PLACE);
        list_dump (list, err);

        return POISON;
    }
    else if (list->size == 0)
    {
        fprintf (stderr, "REMOVE_ERROR: remove from empty list");

        set_error_bit (err, LIST_REMOVE_FROM_EMPTY_LIST);
        list_dump (list, err);

        return POISON;
    }

    check_list (list, err);

    list_elem_t return_value = remove_elem->data;
    remove_elem->prev->next = remove_elem->next;
    remove_elem->next->prev = remove_elem->prev;

    (list->size)--;

    check_list (list, err);

    return return_value;
}

List_elem *find_elem (List *list, int phys_index, unsigned int *err)
{
    List_elem *desired_elem = list->zero_elem;

    while (phys_index--)
    {
        desired_elem = desired_elem->next;
    }

    return desired_elem;
}

void list_dtor (List *list, unsigned int *err)
{
    if (list != nullptr && list->zero_elem != nullptr)
    {
        list_free (list);
    }
    else
    {
        printf ("\nnothing to dtor\n");
    }
}

void list_free (List *list)
{
    List_elem *temp_elem = list->zero_elem->prev;
    List_elem *temp_elem_2 = temp_elem;

    for (;(list->size)-- > 0;)
    {
        temp_elem_2 = temp_elem_2->prev;
        free (temp_elem);
        temp_elem = temp_elem_2;
    }

    free (list->zero_elem);

    list->zero_elem = nullptr;
    list = nullptr;
}

int check_list (List *list, unsigned int *err)
{
 //   int index = list->elems[NULL_ELEM].next;
    int counter = 0;

    /*do
    {
        if (is_bad_ptr (list))
        {
            set_error_bit (err, LIST_BAD_READ_LIST);
            break;
        }
        if (is_bad_ptr (list->elems))
        {
            set_error_bit (err, LIST_BAD_READ_DATA);
            break;
        }
        if (list->size < 0)
        {
            set_error_bit (err, LIST_INCORRECT_SIZE);
        }
        if (list->capacity <= 0)
        {
            set_error_bit (err, LIST_INCORRECT_CAPACITY);
        }

        if (!(*err & LIST_INCORRECT_SIZE) && !(*err & LIST_INCORRECT_CAPACITY))
        {
            while (counter++ < list->size - 1)
            {
                if (list->elems[list->elems[index].next].prev != index)
                {
                    set_error_bit (err, LIST_PREV_NEXT_OP_ERR);
                }
                index = list->elems[index].next;
            }

            counter = 0;
            index = list->free;

            while (counter++ < list->capacity - list->size - 1)
            {
                if (list->elems[index].prev != EMPTY)
                {
                    set_error_bit (err, LIST_FREE_ELEM_NOT_EMPTY);
                }
            }
        }

        #ifdef CANARY_PROT
        if (list->left_canary != CANARY || list->right_canary != CANARY)
        {
            set_error_bit (err, LIST_VIOLATED_LIST);
        }
        if (   *(canary_t *)((char *)list->elems - sizeof (CANARY)) != CANARY
            || *(canary_t)(list->elems + list->capacity) != CANARY)
        {
            set_error_bit (err, LIST_VIOLATED_DATA);
        }
        #endif
    }while(0);
*/
    //if (*err)
    //{
    list_dump (list, err);
    //}
}

void list_dump (List *list, unsigned int *err)
{
    static int PNG_FILE_NUMBER = 1;

    FILE *list_graph = fopen ("list_graph", "w");
    assert (list_graph);

    fprintf (list_log, "<pre>\nlist[%p]\n", list);

    dump_list_members (list, err);
    dump_list_errors  (list, err);

    make_graph (list, list_graph);
    fclose (list_graph);

    char cmd[100] = {};

    sprintf (cmd, "Dot list_graph -T png -o dot%d.png", PNG_FILE_NUMBER);
    printf ("%s", cmd);
    system (cmd);


    fprintf (list_log, "<img src = dot%d.png>", PNG_FILE_NUMBER++);

    fprintf (list_log, "\n\n\n\n\n");

}
/*
struct gv_Node
{
    const char name[GV_MAX_NAME];
    const gv_Node *next;
    const gv_Attributes *attributes;
};

gv_Edge

gv_print_node(gv_Node *node)
{
...
}

struct gv_Attributes
{
    const char *label = nullptr;
    const char *shape;
    const char *style;
};

printf("label = ", attr->label);
printf("label = ", attr->label);
printf("label = ", attr->label);
printf("label = ", attr->label);
printf("label = ", attr->label);

gv_Attributes blue;
blue.shape = "rounded";
...

red

gv_Node node;
// number --> string??
node.attibutes = &blue;


 = {
.shape = "rounded", ""}


struct gv_Attribute
{
    const char *name;
    const char *value;
}

struct gv_Attribute
{
    const char *str;
}
*/

void make_graph (List *list, FILE *list_graph)
{
    int idx = 0;
    List_elem *temp_elem = list->zero_elem;

    fprintf (list_graph, "digraph {\n\tgraph [dpi = 100]\n\t"
                         "splines = \"ortho\";\n\t"
                         "rankdir = LR;\n\t"
                         "header [shape = record,  style = \"filled\", fillcolor = \"grey73\","
                         "label = \"idx\\n | data \\n | next \\n | prev \"];\n");

    while (idx <= list->size)
    {
        if (temp_elem->data == POISON)
        {
            fprintf (list_graph, "\tlabel_%d [shape = record, style = \"filled\", fillcolor = \"salmon1\","
                                 "label = \"%d\\n | d[%p]\\n | n[%d]\\n | p[%d]\"];\n ",
                                 idx, temp_elem, temp_elem->data, temp_elem->next, temp_elem->prev);
        }
        else
        {
            fprintf (list_graph, "\tlabel_%d [shape = record, style = \"filled\", fillcolor = \"lightblue\","
                                 "label = \"%p\\n | d[%d]\\n | n[%p]\\n | P [%p]\"];\n ",
                                 idx, temp_elem, temp_elem->data, temp_elem->next, temp_elem->prev);
        }

        idx++;

        temp_elem = temp_elem->next;
    }

    idx = 0;

    fprintf (list_graph, "\t{edge [style = \"invis\", arrowsize = 44, weight = 1000];\n\t");
    fprintf (list_graph, "header->label_%d;\n\t", idx);


    while (idx < list->size)
    {
        fprintf (list_graph, "label_%d->", idx);

        idx++;
    }

    fprintf (list_graph, "label_%d;\n\t}\n", idx);
    fprintf (list_graph, "\tedge [color = \"purple\", weight = 10, penwidth = 10];\n");

    idx = 1;

    while (idx <= list->size)
    {
        fprintf (list_graph, "label_%d->", idx);

        idx++;
    }

    fprintf (list_graph, "label_%d;\n}", 0);
}

void dump_list_members (List *list, unsigned int *err)
{
    fprintf (list_log, "list size id %d\nlist zero elem is [%p]\n",
                       list->size, list->zero_elem);
    dump_elems (list, err);
}

void dump_elems (List *list, unsigned int *err)
{
    /*for (int index = 0; index < list->capacity; index++)
    {
        fprintf (list_log, "idx[%d]\t data [%d]\t next is [%d]\t prev is [%d]\n",
                 index, list->elems[index].data, list->elems[index].next, list->elems[index].prev);
    }*/
}

void dump_list_errors (List *list, unsigned int *err)
{

#define log_error(__error_bit, __msg)        \
    if (*err & __error_bit)                  \
    {                                        \
        fprintf (list_log, __msg);           \
    }

    do
    {
        if (*err & LOG_FOPEN_FAIL)
        {
            fprintf (stderr, "opening of log file failed");
            break;
        }

        log_error(LIST_ALLOCATION_FAIL, "calloc failed");
        log_error(LIST_BAD_READ_LIST, "list is a bad ptr");
        log_error(LIST_BAD_READ_DATA, "list elems is a bad ptr");
        log_error(LIST_INCORRECT_CAPACITY, "capacity is incorrect (<=0)");
        log_error(LIST_PREV_NEXT_OP_ERR, "next element of previous is not equal to original");
        log_error(LIST_FREE_ELEM_NOT_EMPTY, "free element is not empty");
        log_error(LIST_VIOLATED_LIST, "access rights of list are invaded");
        log_error(LIST_VIOLATED_DATA, "access rights of list data are invaded");

    } while (0);

#undef log_error
}



#endif /* LIST_H */

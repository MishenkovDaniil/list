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
static const int NULL_ELEM = 0; // 0 nullptr NULL
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
    LIST_POP_FROM_EMPTY_LIST    = 0x1 << 9,
    LIST_PREV_NEXT_OP_ERR       = 0x1 << 10,
    LIST_FREE_ELEM_NOT_EMPTY    = 0x1 << 11,
    LIST_VIOLATED_LIST          = 0x1 << 12,
    LIST_VIOLATED_DATA          = 0x1 << 13,
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
    int free = 0;
    int size = 0;
    int capacity = 0;

    List_elem *elems = nullptr;

    #ifdef CANARY_PROT
    canary_t right_canary = CANARY;
    #endif
};


void list_ctor         (List *list, int capacity,                     unsigned int *err = &ERRNO);
void fill_list         (List *list, int start,                        unsigned int *err);
void list_dtor         (List *list,                                   unsigned int *err = &ERRNO);
int list_insert        (List *list, int put_place, list_elem_t value, unsigned int *err = &ERRNO);
int check_list         (List *list,                                   unsigned int *err);
list_elem_t list_pop   (List *list, int pop_place,                    unsigned int *err = &ERRNO);
void list_dump         (List *list,                                   unsigned int *err);
void dump_list_members (List *list,                                   unsigned int *err);
void dump_elems        (List *list,                                   unsigned int *err);
void dump_list_errors  (List *list,                                   unsigned int *err);
void make_graph        (List *list, FILE *list_graph);
void list_realloc      (List *list, int previous_capacity,            unsigned int *err = &ERRNO);
int linearize_list     (List *list,                                   unsigned int *err = &ERRNO, const int phys_index = 0);
void list_free         (List *list);
void set_error_bit     (unsigned int *error, int bit);
int find_logic_number (List *list, int phys_index, unsigned int *err = &ERRNO);
int find_number (List *list, int phys_index, unsigned int *err = &ERRNO);

void set_error_bit (unsigned int *error, int bit)
{
    *error |= bit;
}

void list_ctor (List *list, int capacity, unsigned int *err)
{
    assert (list);

    if (capacity <= 0 || capacity > MAX_CAPACITY)
    {
        set_error_bit (err, LIST_INCORRECT_CAPACITY);
    }
    else
    {
        list->capacity = capacity;

        list_realloc (list, 0);

        list->head = list->tale = list->size = INITIAL;

        list->elems[NULL_ELEM].data = list->elems[NULL_ELEM].prev = list->elems[NULL_ELEM].next = INITIAL;

        list->free = 1;

        fill_list (list, INITIAL + 1, err);
    }

    check_list (list, err);
}

int list_insert (List *list, int put_place, list_elem_t value, unsigned int *err)
{
    assert (list);
    assert (err);

    check_list (list, err);

    list_realloc (list, list->capacity);

    int insert_index = list->free;

    int previous_free = list->free;

    list->free = list->elems[previous_free].next;

    if (put_place > MAX_CAPACITY)
    {
        set_error_bit (err, LIST_INCORRECT_INSERT_PLACE);
        check_list (list, err);

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
        set_error_bit (err, LIST_INSERT_ERROR);

        check_list (list, err);

        list->free = previous_free;

        return POISON;
    }
    else if (put_place == list->tale)
    {
        list->elems[insert_index].data = value;
        list->elems[insert_index].prev = list->tale;
        list->elems[insert_index].next = NULL_ELEM;

        list->elems[list->tale].next = insert_index;

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

    check_list (list, err);

    return insert_index;
}

// inline function??


list_elem_t list_pop (List *list, int pop_place, unsigned int *err)
{
// size capacity size_t
    if (!(pop_place > 0 && pop_place < list->capacity) || (list->elems[pop_place].data == POISON))
    {
        fprintf (stderr, "POP_ERROR: incorrect pop place");

        set_error_bit (err, LIST_INCORRECT_REMOVE_PLACE);
        list_dump (list, err);

        return POISON;
    }
    else if (list->size == 0)
    {
        fprintf (stderr, "POP_ERROR: pop from empty list");

        set_error_bit (err, LIST_POP_FROM_EMPTY_LIST);
        list_dump (list, err);

        return POISON;
    }

    check_list (list, err);

    int return_value = list->elems[pop_place].data;

    if (pop_place == list->head)
    {
        list->head = list->elems[pop_place].next;
        list->elems[list->head].prev = NULL_ELEM;
    }
    else if (pop_place == list->tale)
    {
        list->tale = list->elems[pop_place].prev;
        list->elems[pop_place].next = list->free;
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

    check_list (list, err);

    return return_value;
}

void list_realloc (List *list, int previous_capacity, unsigned int *err)
{
    if (!(previous_capacity) || list->size >= list->capacity - 1)
    {
        list->capacity += previous_capacity;

        #ifdef CANARY_PROT
        list->elems = (List_elem *)realloc (list->elems, sizeof (List_elem) * list->capacity + sizeof (CANARY) * CANARY_NUM); //check if nullptr
        assert (list->elems);

        if (!(previous_capacity))
        {
            *(canary_t *)(list->elems) = CANARY;
        }

        list->elems = (List_elem *)((char *)list->elems + sizeof (CANARY));

        *(canary_t *)(list->elems + list->capacity) = CANARY;
        #else
        list->elems = (List_elem *)realloc (list->elems, list->capacity * sizeof (List_elem)); //check if nullptr
        assert (list->elems);
        #endif

        if (!(previous_capacity))
        {
            list->free = 1;
            fill_list (list, 1, err);
        }
        else
        {
            list->free = previous_capacity;
            fill_list (list, previous_capacity, err);
        }
    }
}

int find_logic_number (List *list, int phys_index, unsigned int *err)
{
    printf ("the function will be working too long, do you really want to call it (ALL INDEX WILL BE INVALID THEN)? (yes/no)");

    const int answer_size = 5;
    char status[answer_size] = {};

    scanf ("%s", status);

    if (stricmp (status, "yes") == 0)
    {
        int desired_index = linearize_list (list, err, phys_index);

        if (desired_index)
        {
            return desired_index;
        }
    }
    printf ("this element is empty or null");
}

int find_number (List *list, int phys_index, unsigned int *err)
{
    printf ("the function will be working too long, do you really want to call it (ALL INDEX WILL BE INVALID THEN)? (yes/no)");

    const int answer_size = 5;
    char status[answer_size] = {};

    scanf ("%s", status);

    if (stricmp (status, "no") == 0)
    {
        int desired_index = linearize_list (list, err, phys_index);

        if (desired_index)
        {
            return desired_index;
        }
    }
    printf ("this element is empty or null");
}

int linearize_list (List *list, unsigned int *err, const int seek_index)
{
    //no additional memory
    List_elem *temp_elems = (List_elem *)calloc (list->capacity, sizeof (List_elem));
    assert (temp_elems);

    if (temp_elems == nullptr)
    {
        fprintf (stderr, "ERROR: allocation failed");
        set_error_bit (err, LIST_ALLOCATION_FAIL);

        return *err;
    }

    int phys_index  = list->head;
    int logic_index = 1;
    int desired_logic_index = 0;

    while (logic_index <= list->size)
    {
        if (seek_index > 0 && phys_index == seek_index)
        {
            desired_logic_index = logic_index;
        }

        temp_elems[logic_index].data = list->elems[phys_index].data;
        temp_elems[logic_index].next = logic_index + 1;
        temp_elems[logic_index].prev = logic_index - 1;

        phys_index = list->elems[phys_index].next;
        logic_index++;
    }

    temp_elems[logic_index - 1].next = NULL_ELEM;

    list->free = logic_index;

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

    free (list->elems);
    list->elems = temp_elems;

    check_list (list, err);

    return desired_logic_index;
}

void fill_list (List *list, int start, unsigned int *err)
{
    for (int index = start; index < list->capacity; index++)
    {
        list->elems[index].data = POISON;
        list->elems[index].prev = EMPTY;

        if (index == list->capacity - 1)
        {
            list->elems[index].next = NULL_ELEM;
        }
        else
        {
            list->elems[index].next = index + 1;
        }
    }
}

void list_dtor (List *list, unsigned int *err)
{
    if (list != nullptr && list->elems != nullptr)
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
    #ifdef CANARY_PROT
    list->elems = (List_elem *)((char *)list->elems - sizeof (CANARY));
    #endif

    free (list->elems);

    list->elems = nullptr;
    list = nullptr;
}

int check_list (List *list, unsigned int *err)
{
    int index = list->head;
    int counter = 0;

    do
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
    fprintf (list_graph, "digraph {\n\tgraph [dpi = 100]\n\t"
                         "splines = \"ortho\";\n\t"
                         "rankdir = LR;\n\t"
                         "header [shape = record,  style = \"filled\", fillcolor = \"grey73\","
                         "label = \"idx\\n | data \\n | next \\n | prev \"];\n");
    while (idx < list->capacity)
    {
        if (list->elems[idx].prev == -1)
        {
            fprintf (list_graph, "\tlabel_%d [shape = record, style = \"filled\", fillcolor = \"salmon1\","
                                 "label = \"%d\\n | d[%p]\\n | n[%d]\\n | p[%d](empty)\"];\n ",
                                 idx, idx, list->elems[idx].data, list->elems[idx].next, list->elems[idx].prev);
        }
        else
        {
            fprintf (list_graph, "\tlabel_%d [shape = record, style = \"filled\", fillcolor = \"lightblue\","
                                 "label = \"%d\\n | d[%d]\\n | n[%d]\\n | P [%d]\"];\n ",
                                 idx, idx, list->elems[idx].data, list->elems[idx].next, list->elems[idx].prev);
        }
        idx++;
    }

    idx = 0;
    fprintf (list_graph, "\t{edge [style = \"invis\", arrowsize = 44, weight = 1000];\n\t");

    fprintf (list_graph, "header->label_%d;\n\t", idx);
    while (idx < list->capacity - 1)
    {
        fprintf (list_graph, "label_%d->", idx);
        idx++;
    }
    fprintf (list_graph, "label_%d;\n\t}\n", idx);

    fprintf (list_graph, "\tedge [color = \"purple\", weight = 10, penwidth = 10];\n\t");
    int counter = 0;
    idx = list->head;

    while (counter++ < list->size)
    {
        fprintf (list_graph, "\tlabel_%d->label_%d;\n", idx, list->elems[idx].next);
        idx = list->elems[idx].next;
    }
    fprintf (list_graph, "}");
}

void dump_list_members (List *list, unsigned int *err)
{
    fprintf (list_log, "list head is %d\nlist tale is %d\n"
                       "list size id %d\nlist capacity id %d\n"
                       "list free is %d\nlist elems is [%p]\n",
                       list->head, list->tale, list->size, list->capacity, list->free, list->elems);
    dump_elems (list, err);
}

void dump_elems (List *list, unsigned int *err)
{
    for (int index = 0; index < list->capacity; index++)
    {
        fprintf (list_log, "idx[%d]\t data [%d]\t next is [%d]\t prev is [%d]\n",
                 index, list->elems[index].data, list->elems[index].next, list->elems[index].prev);
    }
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
    // log err or list err
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "glthreads.h"

static void *
glthread_malloc(size_t size){
    void *p;

    if ((p = malloc(size)) == NULL){
	perror("malloc");
	exit(-1);
    }

    return p;
}

gldll *
glthread_create_list(int (*key_match_cb)(void *, void *),
		     int (*compare_cb)(void *, void *),
		     void (*free_cb)(void **, void *),
		     char *offset){
    gldll *new_gllist;

    new_gllist = glthread_malloc(sizeof(gldll));
    new_gllist->head = NULL;
    new_gllist->key_match_cb = key_match_cb;
    new_gllist->compare_cb = compare_cb;
    new_gllist->free_cb = free_cb;
    new_gllist->offset = offset;

    return new_gllist;
}

/*
 * The address value of the glue node minus its offset points
 * to the beginning of the application-defined structure.
 */
void *
glthread_get_app_structure(gldll *gllist, glthread_node *node){
    if (gllist != NULL)
	return (void *)((char *)node - gllist->offset);
    else
	return NULL;
}

int
glthread_list_length(gldll *gllist){
    glthread_node *node;
    int count = 0;

    if (!gllist)
	return -1;

    node = gllist->head;
    while(node){
	count++;
	node = node->next;
    }

    return count;
}

void
glthread_check_list_len(gldll *gllist, int expected){
    int len;

    if (!gllist || !gllist->head){
	fprintf(stderr,
		"the passed gldll is null. do nothing\n");
	return;
    }

    len = glthread_list_length(gllist);
    if (len != expected){
	fprintf(stderr, "expected the list length to be %d, but it was %d.\n",
		expected, len);
	exit(-1);
    }
}

void
glthread_insert_entry(gldll *gllist, glthread_node *new){
    glthread_node *prev = NULL, *node = NULL;

    if (!gllist || !new)
	return;

    node = gllist->head;

    if (!node){
	gllist->head = new;
	return;
    }

    while(node){
	prev = node;
	node = node->next;
    }

    prev->next = new;
    new->prev = prev;
}

void*
glthread_get_entry(gldll *gllist, void *key){
    glthread_node *node;
    void *app_entry;

    if (!gllist || !key || !gllist->key_match_cb)
	return NULL;

    for(node = gllist->head; node; node = node->next){
	app_entry = glthread_get_app_structure(gllist, node);
	if (gllist->key_match_cb(app_entry, key) == 0){
	    return app_entry;
	}
    }

    return NULL;
}

glthread_node *
glthread_get_first_entry(gldll *gllist){
    glthread_node *first_node;

    if (!gllist || !gllist->head)
	return NULL;

    first_node = gllist->head;

    /* Reconnect when subsequent nodes exist */
    if (first_node->next != NULL){
	gllist->head = first_node->next;
	first_node->next->prev = NULL;
    }

    return first_node;
}


int
glthread_compare_entries(gldll *gllist, void *entry1, void *entry2){
    if (!gllist || !entry1 || !entry2)
	return -1;

    if (!gllist->compare_cb)
	return -1;

    return gllist->compare_cb(entry1, entry2);
}

/* Remove one entry from multiple lists */
int
glthread_remove_entry_from_lists(gldll **gllist_array,
				 uintptr_t array_index, void *key){
    gldll *gllist;
    void *entry;

    if (!gllist_array || !key || array_index < 0)
	return -1;

    gllist = gllist_array[array_index];
    if (!gllist->head || !gllist->key_match_cb || !gllist->free_cb)
	return -1;

    if((entry = glthread_get_entry(gllist, key)) != NULL){
	gllist->free_cb((void **) gllist_array, entry);
	return 0;
    }

    return -1;
}

/*
 * Remove all entries from one list, which propagates to all
 * other lists if necessary.
 */
void
glthread_remove_all_list_entries(gldll **gllist_array,
				 uintptr_t array_index){
    glthread_node *node;
    gldll *target_list;
    void *entry;

    if (!gllist_array || array_index < 0)
	return;

    target_list = gllist_array[array_index];

    if (!target_list)
	return;

    node = target_list->head;

    while(node){
	entry = glthread_get_app_structure(target_list, node);
	if (entry != NULL){
	    target_list->free_cb((void **) gllist_array,
				 entry);
	}
	node = node->next;
    }

    gllist_array[array_index] = NULL;
}

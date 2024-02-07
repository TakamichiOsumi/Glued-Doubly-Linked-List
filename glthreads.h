#ifndef _GLTHREAD_H_
#define _GLTHREAD_H_

/*
 * This macro computes the offset of a given
 * field in a given structure. This is possible
 * since the offset is computed from zero.
 */
#define offsetof(struct_name, field_name)		\
    ((char *)&((struct_name *)0)->field_name)

typedef struct glthread_node {
    struct glthread_node *prev;
    struct glthread_node *next;
} glthread_node;

typedef struct gldll {
    /* The first entry of the list */
    glthread_node *head;

    /* Perform key matching with entry */
    int (*key_match_cb)(void *, void *);

    /* Compare two user-defined data */
    int (*compare_cb)(void *, void *);

    /* Free an entry from multiple glue lists */
    void (*free_cb)(void **, void *);

    /*
     * This is the glue's offset from the top
     * member of application-specific data structure.
     *
     * See the detail in glthread_get_app_structure().
     */
    char *offset;

} gldll;

gldll *glthread_create_list(int (*key_match_cb)(void *, void *),
			    int (*compare_cb)(void *, void *),
			    void (*free_cb)(void**, void *),
			    char *offset);
int glthread_list_length(gldll *gllist);
void *glthread_get_app_structure(gldll *gllist,
				 glthread_node *node);
void glthread_free_list(gldll *list);
void glthread_check_list_len(gldll *gllist, int expected);
void glthread_insert_entry(gldll *gllist, glthread_node *new);
int glthread_remove_entry_from_lists(gldll **gllist_array,
				     uintptr_t list_index, void *key);
void *glthread_get_entry(gldll *gllist, void *key);
glthread_node *glthread_get_first_entry(gldll *gllist);
int glthread_compare_entries(gldll *gllist, void *entry1,void *entry2);
void glthread_remove_all_list_entries(gldll **gllist_array,
				      uintptr_t array_index);

#endif

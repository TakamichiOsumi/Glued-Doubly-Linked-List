#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glthreads.h"

/*
 * In multi-reference scenario, there should be several
 * data structures that refer to same application data
 * at the same time. In this test program, create up
 * to three lists.
 *
 * References by the three lists are implemented in the
 * app_free_bidirectional_reference_test function.
 */
#define LISTS_TOTAL_NUM 3
#define STRLEN 64

/*
 * Application-specific type for multi-reference scenario.
 *
 * All students data are dummy and written just for learning purpose.
 */
typedef struct MultiRefStudent {

    /* Application data */
    unsigned int id;
    char name[STRLEN];

    /*
     * Sticky gums
     *
     * Define all required glues as one array. This
     * enables an iteration in app_free_MRStudent
     * of all glues and make the code easier to read.
     * Additionally, if this is defined as a pointer of
     * glthread_node, then it'd become hard to calculate
     * the offset of each glue from the struct top member.
     */
    glthread_node glues[LISTS_TOTAL_NUM];

} MultiRefStudent;

static void
app_print_one_MRStudent(MultiRefStudent *mrs){
    printf("id : %d, name : %s\n", mrs->id, mrs->name);
}

static void*
app_malloc(size_t size){
    void *p;

    p = (void *)malloc(size);
    if (p == NULL){
	perror("malloc");
	exit(-1);
    }

    return p;
}

static MultiRefStudent*
app_create_MRStudent(unsigned int id, char *name){
    MultiRefStudent *mrs;

    mrs = app_malloc(sizeof(MultiRefStudent));
    mrs->id = id;
    strncpy(mrs->name, name, STRLEN);
    mrs->glues[0].prev = mrs->glues[0].next = NULL;
    mrs->glues[1].prev = mrs->glues[1].next = NULL;
    mrs->glues[2].prev = mrs->glues[2].next = NULL;

    return mrs;
}

#define GET_NEW_MRSTUDENT_GLUE_ADDR0(ID, NAME)	\
    (&app_create_MRStudent(ID, NAME)->glues[0])
#define GET_NEW_MRSTUDENT_GLUE_ADDR1(ID, NAME)	\
    (&app_create_MRStudent(ID, NAME)->glues[1])
#define GET_NEW_MRSTUDENT_GLUE_ADDR2(ID, NAME)	\
    (&app_create_MRStudent(ID, NAME)->glues[2])

/* key_match_cb callback function */
static int
app_search_MRStudent_by_id(void *student, void *target_id){
    MultiRefStudent *mrs = student;
    uintptr_t id = (uintptr_t) target_id;

    if (mrs->id == id)
	return 0;
    else
	return -1;
}

/* free_cb callback function */
static void
app_free_MRStudent(void **lists, void *entry){
    MultiRefStudent *mrs = entry;
    gldll *gllist;
    glthread_node *reconnected;
    int index = 0;

    /* Iterate all lists and reconnect each glue one by one */
    for (; index < LISTS_TOTAL_NUM; index++){
	gllist = lists[index];
	if (gllist != NULL && gllist->head != NULL){
	    reconnected = &mrs->glues[index];
	    if(reconnected->prev == NULL && reconnected->next == NULL){
		/*
		 * Skip any glue that points to no other node.
		 * This means this offset glue doesn't make any linked list.
		 */
		continue;
	    }else if (reconnected->prev == NULL && reconnected->next != NULL){
		gllist->head = reconnected->next;
		/*
		 * Make the next node's 'prev' point to NULL, since the
		 * reconnected node will be freed and the pointer to it will be
		 * invalid.
		 */
		reconnected->next->prev = NULL;
	    }else if (reconnected->prev != NULL && reconnected->next == NULL){
		reconnected->prev->next = NULL;
	    }else if (reconnected->prev != NULL && reconnected->next != NULL){
		reconnected->prev->next = reconnected->next;
		reconnected->next->prev = reconnected->prev;
	    }
	}
    }

    free(mrs);
}

static void
app_print_all_MRStudents(gldll *gllist){
    glthread_node *node = gllist->head;
    MultiRefStudent *mrs;

    if (!node)
	return;

    while(node){
	mrs = glthread_get_app_structure(gllist, node);
	app_print_one_MRStudent(mrs);
	node = node->next;
    }
}

static int
app_compare_MRStudents(void *data1, void *data2){
    MultiRefStudent *s1 = data1, *s2 = data2;

    if (s1->id == s2->id && 
	strncmp(s1->name, s2->name, STRLEN) == 0)
	return 0;
    else
	return -1;
}

static void
app_compare_entries_test(void){
    gldll **gllist_array = app_malloc(sizeof(gldll *) * LISTS_TOTAL_NUM);
    MultiRefStudent *entry1, *entry2;

    gllist_array[0] = glthread_create_list(app_search_MRStudent_by_id,
					   app_compare_MRStudents,
					   app_free_MRStudent,
					   offsetof(MultiRefStudent, glues));
    gllist_array[1] = gllist_array[2] = NULL;

    /* Build one list */
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(1, "Mike"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(2, "John"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(3, "Lian"));

    /* Execute the comparison test */
    entry1 = glthread_get_entry(gllist_array[0], (void *) 1);
    entry2 = glthread_get_entry(gllist_array[0], (void *) 2);

    if (glthread_compare_entries(gllist_array[0], entry1, entry1) != 0)
	exit(-1);

    if (glthread_compare_entries(gllist_array[0], entry1, entry2) != -1)
	exit(-1);

    /* Clean up */
    glthread_remove_all_list_entries(gllist_array, 0);

    free(gllist_array);
}

static void
app_free_single_entry_test(void){
    gldll **gllist_array = app_malloc(sizeof(gldll *) * LISTS_TOTAL_NUM);

    gllist_array[0] = glthread_create_list(app_search_MRStudent_by_id,
					   app_compare_MRStudents,
					   app_free_MRStudent,
					   offsetof(MultiRefStudent, glues));
    gllist_array[1] = gllist_array[2] = NULL;

    /* Build one list */
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(1, "Mike"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(2, "John"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(3, "Lian"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(4, "William"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(5, "Jackson"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(6, "James"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(7, "Edward"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(8, "Olivia"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(9, "Emma"));

    /* Remove entries one by one */
    glthread_check_list_len(gllist_array[0], 9);
    glthread_remove_entry_from_lists(gllist_array, 0, (void *) 9);
    glthread_check_list_len(gllist_array[0], 8);
    glthread_remove_entry_from_lists(gllist_array, 0, (void *) 1);
    glthread_check_list_len(gllist_array[0], 7);
    glthread_remove_entry_from_lists(gllist_array, 0, (void *) 5);
    glthread_check_list_len(gllist_array[0], 6);

    /* Debug */
    app_print_all_MRStudents(gllist_array[0]);

    /* Clean up */
    glthread_remove_all_list_entries(gllist_array, 0);

    /* Were all entries deleted ? */
    glthread_check_list_len(gllist_array[0], 0);

    free(gllist_array);
}

static void
app_free_unidirectional_reference_test(void){
    gldll **gllist_array = app_malloc(sizeof(gldll *) * LISTS_TOTAL_NUM);
    MultiRefStudent *entry;

    gllist_array[0] = glthread_create_list(app_search_MRStudent_by_id,
					   app_compare_MRStudents,
					   app_free_MRStudent,
					   offsetof(MultiRefStudent, glues[0]));
    gllist_array[1] = glthread_create_list(app_search_MRStudent_by_id,
					   app_compare_MRStudents,
					   app_free_MRStudent,
					   offsetof(MultiRefStudent, glues[1]));
    gllist_array[2] = NULL;

    /* Build the 1st list */
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(1, "Mike"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(2, "John"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(3, "Lian"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(4, "William"));

    /*
     * Build the 2nd list
     *
     * Make gllist_array[1] refer to three nodes of gllist_array[0].
     * The three nodes will be inserted at the beginning of the list,
     * in the middle of other nodes and lastly at the end of the list.
     */

    /* Fetch the first node */
    entry = glthread_get_entry(gllist_array[0], (void *) 2);
    glthread_insert_entry(gllist_array[1], &entry->glues[1]);

    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(5, "Jackson"));
    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(6, "James"));

    /* Fetch another node and insert it to gllist_array[1] */
    entry = glthread_get_entry(gllist_array[0], (void *) 1);
    glthread_insert_entry(gllist_array[1], &entry->glues[1]);

    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(7, "Edward"));
    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(8, "Olivia"));
    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(9, "Emma"));

    /* Fetch the last node */
    entry = glthread_get_entry(gllist_array[0], (void *) 3);
    glthread_insert_entry(gllist_array[1], &entry->glues[1]);

    /* Execute the main test */
    glthread_check_list_len(gllist_array[1], 8);
    glthread_remove_all_list_entries(gllist_array, 0);
    glthread_check_list_len(gllist_array[1], 5);

    /* Clean up */
    glthread_remove_all_list_entries(gllist_array, 1);

    free(gllist_array);
}

static void
app_free_bidirectional_reference_test(void){
    gldll **gllist_array = app_malloc(sizeof(gldll *) * LISTS_TOTAL_NUM);
    MultiRefStudent *entry;

    gllist_array[0] = glthread_create_list(app_search_MRStudent_by_id,
					   app_compare_MRStudents,
					   app_free_MRStudent,
					   offsetof(MultiRefStudent, glues[0]));
    gllist_array[1] = glthread_create_list(app_search_MRStudent_by_id,
					   app_compare_MRStudents,
					   app_free_MRStudent,
					   offsetof(MultiRefStudent, glues[1]));
    gllist_array[2] = glthread_create_list(app_search_MRStudent_by_id,
					   app_compare_MRStudents,
					   app_free_MRStudent,
					   offsetof(MultiRefStudent, glues[2]));
    /* Build the 1st list */
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(1, "Mike"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(2, "John"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(3, "Lian"));
    glthread_insert_entry(gllist_array[0],
			  GET_NEW_MRSTUDENT_GLUE_ADDR0(4, "William"));

    /*
     * Build the 2nd list
     *
     * Make gllist_array[1] refer to three nodes of gllist_array[0].
     * The three nodes will be inserted at the beginning of the list,
     * in the middle of other nodes and lastly at the end of the list.
     */

    /* Fetch the first node */
    entry = glthread_get_entry(gllist_array[0], (void *) 2);
    glthread_insert_entry(gllist_array[1], &entry->glues[1]);

    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(5, "Jackson"));
    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(6, "James"));

    /* Fetch another node and insert it to gllist_array[1] */
    entry = glthread_get_entry(gllist_array[0], (void *) 1);
    glthread_insert_entry(gllist_array[1], &entry->glues[1]);

    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(7, "Edward"));
    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(8, "Olivia"));
    glthread_insert_entry(gllist_array[1],
			  GET_NEW_MRSTUDENT_GLUE_ADDR1(9, "Emma"));

    /* Fetch the last node */
    entry = glthread_get_entry(gllist_array[0], (void *) 3);
    glthread_insert_entry(gllist_array[1], &entry->glues[1]);

    /*
     * Build the 3rd list
     *
     * Make gllist_array[2] refer to one nodes of gllist_array[0].
     * Make gllist_array[2] refer to one nodes of gllist_array[1].
     */
    glthread_insert_entry(gllist_array[2],
			  GET_NEW_MRSTUDENT_GLUE_ADDR2(10, "George"));
    entry = glthread_get_entry(gllist_array[0], (void *) 3);
    glthread_insert_entry(gllist_array[2], &entry->glues[2]);
    entry = glthread_get_entry(gllist_array[1], (void *) 7);
    glthread_insert_entry(gllist_array[2], &entry->glues[2]);
    glthread_insert_entry(gllist_array[2],
			  GET_NEW_MRSTUDENT_GLUE_ADDR2(11, "Noah"));

    /*
     * Create more complicated references
     *
     * Make gllist_array[0] refer to two nodes of gllist_array[2].
     * Make gllist_array[0] refer to one node of gllist_array[1].
     */
    entry = glthread_get_entry(gllist_array[2], (void *) 10);
    glthread_insert_entry(gllist_array[0], &entry->glues[0]);
    entry = glthread_get_entry(gllist_array[2], (void *) 11);
    glthread_insert_entry(gllist_array[0], &entry->glues[0]);

    entry = glthread_get_entry(gllist_array[1], (void *) 5);
    glthread_insert_entry(gllist_array[0], &entry->glues[0]);

    /* Basic checks of existing number of entries before the test */
    glthread_check_list_len(gllist_array[0], 7);
    glthread_check_list_len(gllist_array[1], 8);
    glthread_check_list_len(gllist_array[2], 4);

    /* Execute the main test */
    glthread_remove_all_list_entries(gllist_array, 0);
    app_print_all_MRStudents(gllist_array[1]);
    glthread_check_list_len(gllist_array[1], 4);
    app_print_all_MRStudents(gllist_array[2]);
    glthread_check_list_len(gllist_array[2], 1);
    glthread_remove_all_list_entries(gllist_array, 2);
    glthread_check_list_len(gllist_array[1], 3);

    /* Clean up */
    glthread_remove_all_list_entries(gllist_array, 1);
    glthread_check_list_len(gllist_array[0], 0);
    glthread_check_list_len(gllist_array[1], 0);
    glthread_check_list_len(gllist_array[2], 0);

    free(gllist_array);
}

int
main(int argc, char **argv){

    app_compare_entries_test();
    app_free_single_entry_test();
    app_free_unidirectional_reference_test();
    app_free_bidirectional_reference_test();

    return 0;
}

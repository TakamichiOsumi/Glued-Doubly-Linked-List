# Glued Doubly Linked List (Glthreads)

Rewrite the traditional doubly linked list based on the glue concept, for learning. Glued DLL node does not contain a pointer to application data in its definition, unlike the node of traditional linked list. The glue node is just a set of pointers which indicates its previous and next nodes and is attached as application data structure member.

```c:glthreads.h
typedef struct glthread_node {
    struct glthread_node *prev;
    struct glthread_node *next;
} glthread_node;
```

```c:application.c
typedef struct ApplicationData {
    /* define some application specific data */
    unsigned int id;
    char name[64];
    /* glue node */
    glthread_node glue;
} ApplicationData;
```

Application data can be obtained from the `glthread_get_app_structure` function that calculates the application data address with the glue offset.

```c:application.c
ApplicationData *data = (ApplicationData *) glthread_get_app_structure(glued_list, node);
```

## Benefits

One can enjoy the main benefits of glthreads in the multi-reference scenario. It is the case when multiple data structures (like linked list) can refer to same application data simultaneously. Firstly, the number of system call to allocate memory (e.g. malloc) can be reduced compared to the traditional DLL, since all the glue nodes are allocated as the application data member while the traditional DLL needs to allocate node memory per number of data strucutres for references. In addition, glthreads has lower time complexity of data deletion than traditional DLL. One iteration of application data structure, reconnection and free of the data suffice for the glthreads, which leads to O(n) plus O(1) multiplied by the number of data structures. On the other hand, traditional linked list requires to iterate all the data structures, find the corresponding nodes and delete all of those. This leads to O(n) multiplied by the number of data structure.

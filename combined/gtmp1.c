#include <omp.h>
#include "gtmp.h"
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

struct tree_node_t {
    struct tree_node_t* parent;
    int id;
    // struct tree_node_t* children[4];
    int num_children;
    bool sense;
    int count;
};

struct tree_t {
    struct tree_node_t* root;
};

struct tree_node_t** entry_arr;


void gtmp_init(int num_threads) {
    if (num_threads < 1) {
        printf("Invalid number of threads\n");
        exit(1);
    }
    entry_arr = calloc(num_threads, sizeof(struct tree_node_t*));
    struct tree_node_t* curr_parent;
    printf("Entering tree construction\n");
    for (int i = 0; i < num_threads; i++) {
        struct tree_node_t* curr_node = (struct tree_node_t*)malloc(sizeof(struct tree_node_t));
        if (i == 0) {
            curr_node->id = i;
            // printf("curr_node %p id is %d\n", curr_node, i);
            curr_node->parent = NULL;
            // printf("curr_node %p parent is null\n", curr_node);
            curr_node->count = 0;
            curr_node->num_children = 0;
            // printf("curr_node %p count is %d\n", curr_node, curr_node->count);
            curr_node->sense = false;
            // printf("curr_node %p sense is %d\n", curr_node, curr_node->sense);
            // curr_node->wake_up = 0;
            curr_parent = curr_node;
            entry_arr[i] = curr_node;
            // printf("entry_arr[0] %p\n", entry_arr[0]);

            printf("Created root node\n");
        }
        else {
            if (curr_parent->count == 4) {
                curr_parent = entry_arr[curr_parent->id + 1];
            }
            curr_node->id = i;
            curr_node->parent = curr_parent;
            curr_node->count = 0;
            curr_node->parent->count = curr_node->parent->count + 1;
            curr_node->parent->num_children = curr_node->parent->num_children + 1;
            curr_node->sense = false;
            // curr_node->wake_up = 0;
            entry_arr[i] = curr_node;
        }
    }
}

void gtmp_barrier() {
     int num_levels = (int)ceil((log2(omp_get_num_threads() - 1) / 2));
     int curr_thread_level = (int)ceil((log2((double)omp_get_thread_num()) / 2));
     if (curr_thread_level == 0) {
        curr_thread_level = 1;
     }
    //  printf("thread id %d, num levels %d, curr_thread_level %d\n", omp_get_thread_num(), num_levels, curr_thread_level);
    //  fflush(stdout);
     if (num_levels == curr_thread_level) {
        struct tree_node_t* my_node = entry_arr[omp_get_thread_num()];
        assert(my_node != NULL);
#pragma omp critical 
        {
            my_node->parent->count = my_node->parent->count - 1;
        }
        while (my_node->parent->sense == my_node->sense) {
            // Don't do anything
        }
        my_node->sense = !(my_node->sense);
     }
     else if (omp_get_thread_num() == 0) {
        struct tree_node_t* my_node = entry_arr[omp_get_thread_num()];
        assert(my_node != NULL);
        while (my_node->count != 0) {
            // Don't do anything
        }
#pragma omp critical
        {
            my_node->sense = !(my_node->sense);
            my_node->count = my_node->num_children;
        }
     }
     else {
        struct tree_node_t* my_node = entry_arr[omp_get_thread_num()];
        assert(my_node != NULL);
        while (my_node->count != 0) {
            // Don't do anything
            // printf("Waiting on middle count to be updated from children\n");
            // fflush(stdout);
        }
#pragma omp critical 
        {
            my_node->parent->count = my_node->parent->count - 1;
        }
        while (my_node->parent->sense == my_node->sense) {
            // Don't do anything
            // printf("Waiting on parent to update\n");
            // fflush(stdout);
        }
#pragma omp critical
        {
            my_node->sense = !(my_node->sense);
            my_node->count = my_node->num_children;
        }
     }
}

void gtmp_finalize() {
    for (int i = 0; i < omp_get_num_threads(); i++) {
        free(entry_arr[i]);
    }
}


#include <omp.h>
#include "gtmp.h"
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

struct tree_node_t {
    struct tree_node_t* parent;
    int id;
    int children[4];
    int num_children;
    bool sense;
    int count;
    int parent_rep;
};

struct tree_t {
    struct tree_node_t* root;
};

struct tree_node_t** entry_arr;
int num_threads_global;

// double* time_measurement;
int epochs;

void gtmp_init(int num_threads) {
    if (num_threads < 1) {
        printf("Invalid number of threads\n");
        exit(1);
    }
    num_threads_global = num_threads;
    entry_arr = calloc(num_threads, sizeof(struct tree_node_t*));
    struct tree_node_t* curr_parent;
    for (int i = 0; i < num_threads; i++) {
        struct tree_node_t* curr_node = (struct tree_node_t*)calloc(1, sizeof(struct tree_node_t));
        if (i == 0) {
            curr_node->id = i;
            curr_node->parent = NULL;
            curr_node->count = 0;
            curr_node->num_children = 0;
            curr_node->sense = false;
            curr_node->children[0] = 0;
            curr_node->children[1] = 0;
            curr_node->children[2] = 0;
            curr_node->children[3] = 0;
            curr_node->parent_rep = -1;

            curr_parent = curr_node;
            entry_arr[i] = curr_node;
        }
        else {
            if (curr_parent->count == 4) {
                curr_parent = entry_arr[curr_parent->id + 1];
            }
            curr_node->id = i;
            curr_node->parent = curr_parent;
            curr_node->count = 0;
            curr_node->num_children = 0;
            curr_node->children[0] = 0;
            curr_node->children[1] = 0;
            curr_node->children[2] = 0;
            curr_node->children[3] = 0;
            curr_node->sense = false;

            curr_node->parent->count = curr_node->parent->count + 1;
            curr_node->parent->num_children = curr_node->parent->num_children + 1;
            curr_node->parent_rep = (curr_node->parent->count) - 1;

            entry_arr[i] = curr_node;
            // printf("thread %d index into array %d\n", i, curr_node->parent_rep);
        }
    }
    // printf("children[0] %d children[1] %d children[2] %d children[3] %d\n", entry_arr[0]->children[0], entry_arr[0]->children[1], entry_arr[0]->children[2], entry_arr[0]->children[3]);
    // time_measurement = calloc(num_threads * 100, sizeof(double));
}

void gtmp_barrier() {
     // struct timeval time_curr_finish;
     // double measure;
     struct tree_node_t* my_node = entry_arr[omp_get_thread_num()];
     assert(my_node != NULL);
     if (my_node->num_children == 0) {
        my_node->parent->children[my_node->parent_rep] = 1;
        // printf("thread %d epoch %d made parent 1\n", omp_get_thread_num(), epochs);
        while (my_node->parent->sense == my_node->sense) {
            // Don't do anything
        }

        my_node->sense = !(my_node->sense);
     }
     else if (my_node->id == 0) {
        int check = ((my_node->num_children < 1) || (my_node->children[0] == 1)) && ((my_node->num_children < 2) || (my_node->children[1] == 1)) && ((my_node->num_children < 3) || (my_node->children[2] == 1)) && ((my_node->num_children < 4) || (my_node->children[3] == 1));
        while (check == 0) {
            // Don't do anything
            check = ((my_node->num_children < 1) || (my_node->children[0] == 1)) && ((my_node->num_children < 2) || (my_node->children[1] == 1)) && ((my_node->num_children < 3) || (my_node->children[2] == 1)) && ((my_node->num_children < 4) || (my_node->children[3] == 1));
        }

        for (int i = 0; i < my_node->num_children; i++) {
            my_node->children[i] = 0;
        }

        epochs = epochs + 1;

#pragma omp critical
        {
            my_node->sense = !(my_node->sense);
            // my_node->count = my_node->num_children;
        }
     }
     else {
        int check = ((my_node->num_children < 1) || (my_node->children[0] == 1)) && ((my_node->num_children < 2) || (my_node->children[1] == 1)) && ((my_node->num_children < 3) || (my_node->children[2] == 1)) && ((my_node->num_children < 4) || (my_node->children[3] == 1));
        while (check == 0) {
            // Don't do anything
            // printf("epoch: %d thread %d, Waiting on middle count to be updated from children, %d\n", epochs, omp_get_thread_num(), my_node->children[0]);
            check = ((my_node->num_children < 1) || (my_node->children[0] == 1)) && ((my_node->num_children < 2) || (my_node->children[1] == 1)) && ((my_node->num_children < 3) || (my_node->children[2] == 1)) && ((my_node->num_children < 4) || (my_node->children[3] == 1));
        }

        my_node->parent->children[my_node->parent_rep] = 1;
        while (my_node->parent->sense == my_node->sense) {
            // Don't do anything
            // printf("epoch: %d, thread %d, Waiting on parent to update\n", epochs, omp_get_thread_num());
        }

        for (int i = 0; i < my_node->num_children; i++) {
            my_node->children[i] = 0;
        }

#pragma omp critical
        {
            my_node->sense = !(my_node->sense);
            // my_node->count = my_node->num_children;
        }
     }
    // gettimeofday(&time_curr_finish, NULL);
    // measure = ((double)(time_curr_end.tv_sec * 1000000) + time_curr_end.tv_usec) - ((double)(time_curr_start.tv_sec * 1000000) + time_curr_start.tv_usec);
    // time_measurement[(omp_get_num_threads() * (epochs - 1)) + omp_get_thread_num()] = (double)(time_curr_finish.tv_sec * 1000000) + time_curr_finish.tv_usec;
    // printf("measure for thread %d is %.9f\n", omp_get_thread_num(), measure);
}

void gtmp_finalize() {
    for (int i = 0; i < num_threads_global; i++) {
        free(entry_arr[i]);
    }
    // double final_val = 0;
    // int adds = 0;
    // double min = __DBL_MAX__;
    // for (int i = 0; i < 100; i++) {
    //     for (int j = 0; j < omp_get_num_threads(); j++) {
    //         if (min > time_measurement[(omp_get_num_threads() * i) + j]) {
    //             min = time_measurement[(omp_get_num_threads() * i) + j];
    //         }
    //     }
    //     if (min > 100.0) {
    //         continue;
    //     }
    //     adds = adds + 1;
    //     final_val = final_val + min;
    // }
    // printf("min avg is %.9f with adds %d\n", final_val/adds, adds);
    // double curr_val = 0.0;
    // double variance = 0.0;
    // double total_variance = 0.0;
    // for (int i = 0; i < 100; i++) {
    //     for (int j = 0; j < num_threads_global; j++) {
    //         // printf("thread %d %.9f\n", j, time_measurement[(num_threads_global * i) + j]);
    //         curr_val = curr_val + time_measurement[(num_threads_global * i) + j];
    //     }
    //     curr_val = curr_val/num_threads_global;
    //     for (int j = 0; j < num_threads_global; j++) {
    //         variance = fabs((time_measurement[(num_threads_global * i) + j] - curr_val));
    //     }
    //     variance = variance/num_threads_global;
    //     total_variance = total_variance + variance;
    //     curr_val = 0.0;
    //     variance = 0.0;
    // }
    // printf("variance is %.9f\n", total_variance/100);
    // free(time_measurement);
}


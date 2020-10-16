
/**
 * deadlock_demolition
 * CS 241 - Fall 2020
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {
    pthread_mutex_t mutex;
};

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
graph* graph_ = NULL;
set* visited_nodes_ = NULL;

// bool has_cycle(void* node) {

//     if (visited_nodes_ == NULL) {
//       visited_nodes_ = shallow_set_create();
//     }

//     if (set_contains(visited_nodes_, node)) {
//         visited_nodes_ = NULL;
//         return true;
//     } else {
//       set_add(visited_nodes_, node);
//       vector* neighbor_nodes = graph_neighbors(graph_, node);

//       for (size_t i = 0; i < vector_size(neighbor_nodes); i++) {
//         if (has_cycle(vector_get(neighbor_nodes, i))) {
//           return true;
//         }
//       }
//     }

//     visited_nodes_ = NULL;
//     return false;
// }

/**
 * Checks for a cycle in the resource allocations graph
 **/

bool has_cycle(void* node) {

    if (!visited_nodes_) {
        visited_nodes_ = shallow_set_create();
    }

    if (set_contains(visited_nodes_, node)) {
        return 1;
    }

    else {
        set_add(visited_nodes_, node);
        vector *neighbors = graph_neighbors(graph_, node);

        for (size_t i = 0; i < vector_size(neighbors); i++) {
            if (has_cycle(vector_get(neighbors, i))) {
                return 1;
            }
        }
    }
    free(visited_nodes_);
    visited_nodes_ = NULL;
    return 0;
}

drm_t *drm_init() {
    // create resource allocations graph
    if (!graph_) {
        graph_ = shallow_graph_create();
    }

    drm_t *d = malloc(sizeof(drm_t));

    pthread_mutex_init(&(d->mutex), NULL);

    pthread_mutex_lock(&lock);
    graph_add_vertex(graph_, d);
    pthread_mutex_unlock(&lock);

    return d;
}


/**
 * UNLOCK
 * @return :
*  0 if the specified thread is not able to unlock the given drm.
*  1 if the specified thread is able to unlock the given drm.
**/
int drm_post(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&lock);
    
    if( !graph_contains_vertex(graph_, drm) || !graph_contains_vertex(graph_, thread_id)) {
        pthread_mutex_unlock(&lock);
        return 0;
    }

    if (graph_adjacent(graph_, drm, thread_id)) {
        // Remove the appropriate edge from your Resource Allocation Graph.
        graph_remove_edge(graph_, drm, thread_id);
        pthread_mutex_unlock(&(drm->mutex));
    }

    pthread_mutex_unlock(&lock);
    return 1;
}

/**
 * LOCK
* @return :
*   0 if attempting to lock this drm with the specified thread would cause
*       deadlock - happens if:
*           - a thread trying to lock a mutex it already owns
*           - if the newly added edge creates a cycle in the Resource Allocation Graph
*   1 if the drm can be locked by the specified thread.
*/
int drm_wait(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&lock);

    if (!graph_contains_vertex(graph_, thread_id)) graph_add_vertex(graph_, thread_id);

    if (graph_adjacent(graph_, drm, thread_id)) {
        pthread_mutex_unlock(&lock);
        return 0;

    }
    // create the appropriate edge in your Resource Allocation Graph
    graph_add_edge(graph_, thread_id, drm);

    // if it creates a cycle, remove the edge and return 0
    if (has_cycle(thread_id)) {
        graph_remove_edge(graph_, thread_id, drm);
        pthread_mutex_unlock(&lock);
        return 0;

    } else {

        pthread_mutex_unlock(&lock);
        pthread_mutex_lock(&(drm->mutex));
        pthread_mutex_lock(&lock);

        graph_remove_edge(graph_, thread_id, drm);
        graph_add_edge(graph_, drm, thread_id);

        pthread_mutex_unlock(&lock);
        return 1;
    }
    return 0;
}

void drm_destroy(drm_t *drm) {
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&drm->mutex);

    graph_remove_vertex(graph_, drm);

    free(drm);
}

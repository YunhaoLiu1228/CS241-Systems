/**
 * Parallel Make
 * CS 241 - Fall 2020
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "includes/queue.h"

#include <stdlib.h>
#include <stdio.h>

int parmake(char *makefile, size_t num_threads, char **targets) {
    graph* graph = parser_parse_makefile(makefile, targets);
    queue* bfs = queue_create(0);

    vector* vertices = graph_vertices(graph);
    queue_push(bfs, vector_front(vertices));


    vector_destroy(vertices);
    queue_destroy(bfs);
    graph_destroy(graph);

    return 0;
}

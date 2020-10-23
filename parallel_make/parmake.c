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
    graph* dependency_graph = parser_parse_makefile(makefile, targets);
    queue* bfs = queue_create(0);

    // list of all targets
    vector* graph_targets = graph_vertices(dependency_graph);

    rule_t* rule = (rule_t*) graph_get_vertex_value(dependency_graph, *vector_back(graph_targets));
    //printf("rule target: %s\n", rule->target);

    char* command = malloc(sizeof(char));
    command = *(char**)vector_front(rule->commands);
    //printf("rule command: %s\n", *(char**)vector_front(rule->commands));

    int status;
    if (( status = system(command) != 0 )) {
        return status;
    };

    free(command);
    vector_destroy(graph_targets);
    queue_destroy(bfs);
    graph_destroy(dependency_graph);

    return 0;
}

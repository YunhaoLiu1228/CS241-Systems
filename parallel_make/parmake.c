/**
 * Parallel Make
 * CS 241 - Fall 2020
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "includes/queue.h"
#include "includes/set.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

graph* dependency_graph;
set* visited_nodes = NULL;
queue* rules;
    int failed = 0;

bool has_cycle(char* goal) {

    if (!visited_nodes) {
        visited_nodes = shallow_set_create();
    }

    if (set_contains(visited_nodes, goal)) {
        set_destroy(visited_nodes);
        visited_nodes = NULL;
        return true;
    }

    else {
        set_add(visited_nodes, goal);
        vector* neighbors = graph_neighbors(dependency_graph, goal);

        for (size_t i = 0; i < vector_size(neighbors); i++) {
            if (has_cycle(vector_get(neighbors, i))) {
                vector_destroy(neighbors);
                return true;
            }
        }
        vector_destroy(neighbors);
    }
    set_destroy(visited_nodes);
    
    visited_nodes = NULL;
    return false;
}

/**
 Specifically, you should not try to satisfy a rule if any of the following is true:
*  (X) the rule is a goal rule, but is involved in a cycle, i.e. there exists a path from the rule to itself in the dependency graph
*  (?) the rule is a goal rule, but at least one of its descendants, i.e. any rule in the dependency graph reachable from the goal rule, is involved in a cycle
*  (?) the rule is not a goal rule, and it has no ancestors that are goal rules
*  (?) the rule is not a goal rule, and all of its goal rule ancestors fall under (1) or (2)
**/
bool should_satisfy(char* target) {
  
    vector *neighbors = graph_neighbors(dependency_graph, target);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        char *neighbor = vector_get(neighbors, i);
        //printf("neighbor: %s\n", neighbor);
        

        rule_t *rule_nbr = (rule_t *) graph_get_vertex_value(dependency_graph, neighbor);
        if (!(rule_nbr->state) && should_satisfy(neighbor)) {
            return true;
        }
        rule_nbr->state = 1;
    }

    vector_destroy(neighbors);

    if (failed) return true;

    rule_t *rule = (rule_t *) graph_get_vertex_value(dependency_graph, target);

    if (!(rule->state)) {
        queue_push(rules, rule);
    } else {
        rule->state = 1;
    }


    return false;

}

void* thread_func(void* arg) {
    bool exec = true;
    while (exec) {
        rule_t *rule = queue_pull(rules);
        if (!rule) {
            queue_push(rules, NULL);
            break;
        }
       // printf("rule: %s\n", rule->target);
        
        vector* dependencies = graph_neighbors(dependency_graph, rule->target);
        
        for (size_t i = 0; i < vector_size(dependencies); i++) {
            char* nbr = vector_get(dependencies, i);
            rule_t* r = graph_get_vertex_value(dependency_graph, nbr);

            //if (nbr) printf("nbr: %s\n", nbr);
            //if (r) printf("r: %s\n", r->target);
            if (r->state == 1) {
                //printf("r: %s\n", r->target);
                failed = 1;
                exec = false;
                break;
                
            }
            //     failed = 1;
            //     break;
            // }
        }
        vector_destroy(dependencies);
        if (!exec) break;

        for (size_t i = 0; i < vector_size(rule->commands); i++) {
            if (system(vector_get(rule->commands, i)) != 0) {
                //printf("nooo\n");
                rule->state = 1;
                failed = 1;
                break;
            } else {
                rule->state = 0;
            }
        }
        
    }
    return NULL;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    dependency_graph = parser_parse_makefile(makefile, targets);

    // list of all targets
    vector* graph_targets = graph_neighbors(dependency_graph, "");      // get goal rules
    rules = queue_create(0);

    for (size_t i = 0 ; i < vector_size(graph_targets); i++) {
        char* goal = *(char**)vector_at(graph_targets, i);

        if (has_cycle(goal)) {
            print_cycle_failure(goal);
        }
        else if (!should_satisfy(goal)) {
            failed = 0;
        }

        
    }
    queue_push(rules, NULL);
    
    pthread_t id[num_threads];

    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(&id[i], NULL, thread_func, NULL);
    }

    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(id[i], NULL);
    }
    

    
    vector_destroy(graph_targets);
    queue_destroy(rules);
    graph_destroy(dependency_graph);
    if (visited_nodes) set_destroy(visited_nodes);

    return 0;
}
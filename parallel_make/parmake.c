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

graph* dependency_graph;
set* visited_nodes = NULL;
queue* rules;
    int failed = 0;

bool has_cycle(char* goal) {

    if (!visited_nodes) {
        visited_nodes = shallow_set_create();
    }

    if (set_contains(visited_nodes, goal)) {
        return true;
    }

    else {
        set_add(visited_nodes, goal);
        vector* neighbors = graph_neighbors(dependency_graph, goal);

        for (size_t i = 0; i < vector_size(neighbors); i++) {
            if (has_cycle(vector_get(neighbors, i))) {
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

    // vector *neighbors = graph_neighbors(dependency_graph, target);
    // for (size_t i = 0; i < vector_size(neighbors); i++) {
    //     char *neighbor = vector_get(neighbors, i);

    //     rule_t* rule = (rule_t*) graph_get_vertex_value(dependency_graph, neighbor);
    //     printf("rule target: %s\n", rule->target);

    //     for (size_t j = 0; j < vector_size(rule->commands); j++) {
    //         char* command = strdup( *(char**)vector_front(rule->commands) );
    //         printf("rule goal: %s\n", command);
    //     }
        
    // }
  
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
    
    while (true) {
    rule_t *rule = queue_pull(rules);
    if (!rule) {
      queue_push(rules, NULL);
      break;
    }
    for (size_t i = 0; i < vector_size(rule->commands); i++) {
      if (system(vector_get(rule->commands, i)) != 0) {
        failed = 1;
        break;
      }
    }
    rule->state = 1;
    //pthread_cond_signal(&cv);
  }
    // rule_t* rule;    
    // while (( (rule  = queue_pull(rules)) != NULL )) {
    //     printf("rule target: %s\n", *(char**)vector_get(rule->commands, 0));
    // }


    // rule_t* rule = (rule_t*) graph_get_vertex_value(dependency_graph, *vector_back(graph_targets));
    // rule->data = true;
    // //printf("rule target: %s\n", rule->target);

    // char* goal = malloc(sizeof(char));
    // goal = *(char**)vector_front(rule->commands);
    // //printf("rule goal: %s\n", *(char**)vector_front(rule->commands));

    // int status;
    // if (( status = system(goal) != 0 )) {
    //     return status;
    // };

    // free(goal);
    vector_destroy(graph_targets);
    queue_destroy(rules);
    graph_destroy(dependency_graph);
    if (visited_nodes) set_destroy(visited_nodes);

    return 0;
}

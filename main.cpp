#include "csp.h"
#include <cstdlib>
#include <stdio.h>
#include <time.h>

using three_coloring::Graph;


Graph read_graph(FILE* file = stdin) {
    unsigned n = 0, tmp = 0;
    fscanf(file, "%u\n", &n);
    Graph graph(n);
    for (unsigned i = 0; i < n; ++i) {
        fscanf(file, "%u: ", &tmp);
        graph[i].resize(tmp);
        for (unsigned j = 0; j < tmp; ++j)
            fscanf(file, "%u ", &graph[i][j]);
    }

    fclose(file);
    return graph;
}


int main(int argc, char** argv) {
    srand(time(0));
    FILE* file = stdin;
    if (argc >= 2) {
        const char* filename = argv[1];
        file = fopen(filename, "w");
        if (!file) {
            perror(filename);
            exit(1);
        }
    }
    Graph graph = read_graph(file);
    printf("%d\n", three_coloring::is_three_colorable(graph));
}

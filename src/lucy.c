#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lucy_api.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <base_annotations.h> <output_annotations.h> <output_annotations.c> <input1.c:output1.c> <input2.c:output2.c> ...\n", argv[0]);
        return 1;
    }

    lucy_init();

    const char *base_annotations_path = argv[1];
    const char *output_annotations_h_path = argv[2];
    const char *output_annotations_c_path = argv[3];

    load_extensions(base_annotations_path);

    for (int i = 4; i < argc; i++) {
        char *input_output = strdup(argv[i]);
        char *input_path = strtok(input_output, ":");
        char *output_path = strtok(NULL, ":");
        if (!input_path || !output_path) {
            fprintf(stderr, "Invalid input:output pair: %s\n", argv[i]);
            free(input_output);
            lucy_cleanup();
            return 1;
        }
        if (lucy_process_file(input_path, output_path) != 0) {
            free(input_output);
            lucy_cleanup();
            return 1;
        }
        free(input_output);
    }

    if (lucy_generate_annotations_header(base_annotations_path, output_annotations_h_path) != 0 ||
        lucy_generate_annotations_source(output_annotations_c_path) != 0) {
        lucy_cleanup();
        return 1;
    }

    lucy_cleanup();
    return 0;
}
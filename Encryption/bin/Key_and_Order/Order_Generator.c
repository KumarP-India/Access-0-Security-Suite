/*
* @Author: Prabhas Kumar
* @Assistant: OpenAI ChatGPT 4.0

* @Created: May 29'24
* @Updated: None

* @Project: Access-0-Security-Suite 's Encryption Version 1
* @File: Order_Generator [C script]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#define MAX_FILES 26 // Max number of single-letter file names from 'a' to 'z'

void shuffle(char *array[], size_t n) {
    if (n > 1) {
        size_t i;
        srand((unsigned int) time(NULL));
        for (i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            char *temp = array[j];
            array[j] = array[i];
            array[i] = temp;
        }
    }
}

int main() {
    const char *directory = "../../";
    DIR *dir;
    struct dirent *entry;
    char *filenames[MAX_FILES];
    size_t file_count = 0;

    if ((dir = opendir(directory)) == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".bin")) {
            filenames[file_count] = strdup(entry->d_name);
            file_count++;
        }
    }
    closedir(dir);

    if (file_count == 0) {
        fprintf(stderr, "No .bin files found in the directory\n");
        return EXIT_FAILURE;
    }

    shuffle(filenames, file_count);

    FILE *order_file = fopen("../../order.txt", "w");
    if (order_file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    char new_filename[3] = "a";
    char old_filepath[256];
    char new_filepath[256];

    for (size_t i = 0; i < file_count; i++) {
        snprintf(old_filepath, sizeof(old_filepath), "%s%s", directory, filenames[i]);
        snprintf(new_filepath, sizeof(new_filepath), "%s%s", directory, new_filename);

        if (rename(old_filepath, new_filepath) != 0) {
            perror("rename");
            return EXIT_FAILURE;
        }

        fprintf(order_file, "%s -> %s\n", filenames[i], new_filename);

        new_filename[0]++;
        free(filenames[i]);
    }

    fclose(order_file);

    return EXIT_SUCCESS;
}

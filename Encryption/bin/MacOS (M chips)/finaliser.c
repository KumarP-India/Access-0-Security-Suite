#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>

#define DATA_DIR "../Data"
#define LOG_FILE "./log.order"
#define AES_KEYLEN 32
#define AES_IVLEN 16

void generate_random_string(char *str, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *str++ = charset[index];
    }
    *str = '\0';
}

void get_hidden_input(char *input, size_t len) {
    struct termios oldt, newt;
    printf("Enter custom key: ");
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fgets(input, len, stdin);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    input[strcspn(input, "\n")] = '\0';
    printf("\n");
}

int is_valid_file(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0;
    }
    if (S_ISREG(statbuf.st_mode)) {
        return 1;
    }
    return 0;
}

int encrypt_file(const char *input_path, const char *output_path, uint8_t *key, uint8_t *iv) {
    FILE *input_file = fopen(input_path, "rb");
    FILE *output_file = fopen(output_path, "wb");
    if (!input_file || !output_file) {
        return 0;
    }

    fwrite(iv, 1, AES_IVLEN, output_file);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return 0;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    uint8_t buffer[4096];
    uint8_t ciphertext[4096 + EVP_CIPHER_block_size(EVP_aes_256_cbc())];
    int len, ciphertext_len;

    while ((len = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
        if (EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len, buffer, len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
        fwrite(ciphertext, 1, ciphertext_len, output_file);
    }

    if (EVP_EncryptFinal_ex(ctx, ciphertext, &ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    fwrite(ciphertext, 1, ciphertext_len, output_file);

    EVP_CIPHER_CTX_free(ctx);
    fclose(input_file);
    fclose(output_file);

    return 1;
}

void handle_logging(const char *keypair_name, const char *encrypted_name, const char *original_file_name) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        fprintf(stderr, "Failed to open log file\n");
        return;
    }
    fprintf(log_file, "%s: %s: CUSTOM\n", original_file_name, encrypted_name);
    fclose(log_file);
}

void read_log_file(char ***log_entries, size_t *count) {
    FILE *log_file = fopen(LOG_FILE, "r");
    if (!log_file) {
        *log_entries = NULL;
        *count = 0;
        return;
    }

    char **entries = NULL;
    size_t entry_count = 0;
    char line[512];

    while (fgets(line, sizeof(line), log_file)) {
        entries = realloc(entries, sizeof(char *) * (entry_count + 1));
        entries[entry_count] = strdup(line);
        entry_count++;
    }

    fclose(log_file);
    *log_entries = entries;
    *count = entry_count;
}

int is_in_log_file(const char *filename, char **log_entries, size_t log_count) {
    for (size_t i = 0; i < log_count; ++i) {
        char *entry = strdup(log_entries[i]);
        char *token = strtok(entry, ":");
        if (strcmp(token, filename) == 0) {
            free(entry);
            return 1;
        }
        free(entry);
    }
    return 0;
}

void delete_file(const char *filepath) {
    if (remove(filepath) != 0) {
        fprintf(stderr, "Failed to delete file: %s\n", filepath);
    }
}

void delete_files_in_directory(const char *directory, const char *extension) {
    DIR *dir = opendir(directory);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strstr(entry->d_name, extension)) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);
            delete_file(filepath);
        }
    }

    closedir(dir);
}

int main() {
    srand(time(NULL));

    char custom_key[64];
    get_hidden_input(custom_key, sizeof(custom_key));
    uint8_t key[AES_KEYLEN];
    strncpy((char *)key, custom_key, sizeof(key));

    uint8_t iv[AES_IVLEN];
    if (!RAND_bytes(iv, sizeof(iv))) {
        fprintf(stderr, "Failed to generate IV\n");
        exit(EXIT_FAILURE);
    }

    char **log_entries;
    size_t log_count;
    read_log_file(&log_entries, &log_count);

    DIR *dir = opendir(DATA_DIR);
    if (!dir) {
        fprintf(stderr, "Failed to open directory: %s\n", DATA_DIR);
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strstr(entry->d_name, ".bin")) {
            if (!is_in_log_file(entry->d_name, log_entries, log_count)) {
                char input_path[512];
                snprintf(input_path, sizeof(input_path), "%s/%s", DATA_DIR, entry->d_name);

                char encrypted_name[10];
                generate_random_string(encrypted_name, 9);

                char output_path[512];
                snprintf(output_path, sizeof(output_path), "%s/%s.enc", DATA_DIR, encrypted_name);

                if (encrypt_file(input_path, output_path, key, iv)) {
                    handle_logging(entry->d_name, encrypted_name, input_path);
                } else {
                    fprintf(stderr, "Failed to encrypt file: %s\n", input_path);
                }
            }
        }
    }

    closedir(dir);

    char log_output_path[512];
    snprintf(log_output_path, sizeof(log_output_path), "../../log.order.enc");

    if (!encrypt_file(LOG_FILE, log_output_path, key, iv)) {
        fprintf(stderr, "Failed to encrypt log file\n");
        exit(EXIT_FAILURE);
    }

    delete_files_in_directory(DATA_DIR, ".bin");
    delete_file(LOG_FILE);

    printf("Encryption and cleanup completed successfully.\n");
    return 0;
}

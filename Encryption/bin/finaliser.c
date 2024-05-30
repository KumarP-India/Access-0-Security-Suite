#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

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

// Function to read the custom key from the user
void read_custom_key(uint8_t *key, size_t key_len) {
    char input[65]; // Allow up to 64 characters for the key
    printf("Enter a custom key (up to 64 characters): ");
    fgets(input, sizeof(input), stdin);

    // Ensure the key is the correct length
    memset(key, 0, key_len);
    strncpy((char*)key, input, key_len - 1);
}

// Function to check if a file exists
int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// Function to read a file into a buffer
int read_file(const char *filename, uint8_t **buffer, size_t *length) {
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = malloc(*length);
    if (!*buffer) {
        fclose(file);
        return 0;
    }

    fread(*buffer, 1, *length, file);
    fclose(file);
    return 1;
}

// Function to encrypt data using AES-256
int encrypt_data(uint8_t *plaintext, size_t plaintext_len, uint8_t *key, uint8_t *iv, uint8_t **ciphertext, size_t *ciphertext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    int len;
    *ciphertext = malloc(plaintext_len + AES_BLOCK_SIZE);
    if (!*ciphertext) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    if (EVP_EncryptUpdate(ctx, *ciphertext, &len, plaintext, plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(*ciphertext);
        return 0;
    }
    *ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, *ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(*ciphertext);
        return 0;
    }
    *ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

// Function to get the list of key files not in log.order
int get_key_files_not_in_log(char ***key_files, size_t *count) {
    DIR *dir = opendir(DATA_DIR);
    if (!dir) return 0;

    FILE *log_file = fopen(LOG_FILE, "r");
    if (!log_file) {
        closedir(dir);
        return 0;
    }

    *key_files = NULL;
    *count = 0;
    struct dirent *entry;
    char line[256];

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".bin")) {
            int in_log = 0;
            rewind(log_file);
            while (fgets(line, sizeof(line), log_file)) {
                if (strstr(line, entry->d_name)) {
                    in_log = 1;
                    break;
                }
            }

            if (!in_log) {
                *key_files = realloc(*key_files, (*count + 1) * sizeof(char*));
                (*key_files)[*count] = malloc(strlen(entry->d_name) + 1);
                strcpy((*key_files)[*count], entry->d_name);
                (*count)++;
            }
        }
    }

    closedir(dir);
    fclose(log_file);
    return 1;
}

// Function to handle logging
void handle_logging(const char *keypair_name, const char *encrypted_name, const char *original_file_name, int clear_log) {
    FILE *log_file = fopen(LOG_FILE, clear_log ? "w" : "a");
    if (!log_file) {
        fprintf(stderr, "Failed to open log file\n");
        return;
    }
    fprintf(log_file, "%s: %s: ../Data/%s\n", original_file_name, encrypted_name, keypair_name);
    fclose(log_file);
}

// Function to move the file to parent directory
int move_file_to_parent(const char *filename) {
    char new_path[512];
    snprintf(new_path, sizeof(new_path), "../../%s", filename);
    return rename(filename, new_path);
}

// Main function
int main() {
    srand(time(NULL)); // Seed the random number generator

    uint8_t key[AES_KEYLEN];
    read_custom_key(key, sizeof(key));

    char **key_files;
    size_t key_files_count;
    if (!get_key_files_not_in_log(&key_files, &key_files_count)) {
        fprintf(stderr, "Failed to get key files\n");
        exit(EXIT_FAILURE);
    }

    uint8_t iv[AES_IVLEN];
    if (!RAND_bytes(iv, sizeof(iv))) {
        fprintf(stderr, "Failed to generate IV\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < key_files_count; ++i) {
        char key_file_path[512];
        snprintf(key_file_path, sizeof(key_file_path), "%s/%s", DATA_DIR, key_files[i]);

        uint8_t *key_buffer;
        size_t key_len;
        if (!read_file(key_file_path, &key_buffer, &key_len)) {
            fprintf(stderr, "Failed to read key file: %s\n", key_files[i]);
            continue;
        }

        char encrypted_name[10];
        generate_random_string(encrypted_name, 9);

        uint8_t *ciphertext;
        size_t ciphertext_len;
        if (!encrypt_data(key_buffer, key_len, key, iv, &ciphertext, &ciphertext_len)) {
            fprintf(stderr, "Failed to encrypt key file: %s\n", key_files[i]);
            free(key_buffer);
            continue;
        }

        free(key_buffer);

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s/%s.enc", DATA_DIR, encrypted_name);
        FILE *output_file = fopen(output_path, "wb");
        if (!output_file) {
            fprintf(stderr, "Failed to open output file: %s\n", output_path);
            free(ciphertext);
            continue;
        }

        fwrite(iv, 1, AES_IVLEN, output_file); // Write IV to the beginning of the output file
        fwrite(ciphertext, 1, ciphertext_len, output_file);
        fclose(output_file);
        free(ciphertext);

        handle_logging(key_files[i], encrypted_name, key_files[i], 0);

        if (remove(key_file_path) != 0) {
            fprintf(stderr, "Failed to delete key file: %s\n", key_files[i]);
        }

        free(key_files[i]);
    }
    free(key_files);

    // Encrypt log.order
    FILE *log_file = fopen(LOG_FILE, "rb");
    if (!log_file) {
        fprintf(stderr, "Failed to open log file\n");
        exit(EXIT_FAILURE);
    }

    fseek(log_file, 0, SEEK_END);
    size_t log_size = ftell(log_file);
    rewind(log_file);

    uint8_t *log_buffer = malloc(log_size);
    fread(log_buffer, 1, log_size, log_file);
    fclose(log_file);

    char encrypted_log_name[10];
    generate_random_string(encrypted_log_name, 9);

    uint8_t *log_ciphertext;
    size_t log_ciphertext_len;
    if (!encrypt_data(log_buffer, log_size, key, iv, &log_ciphertext, &log_ciphertext_len)) {
        fprintf(stderr, "Failed to encrypt log file\n");
        free(log_buffer);
        exit(EXIT_FAILURE);
    }

    free(log_buffer);

    char log_output_path[512];
    snprintf(log_output_path, sizeof(log_output_path), "%s.enc", LOG_FILE);
    FILE *encrypted_log_file = fopen(log_output_path, "wb");
    if (!encrypted_log_file) {
        fprintf(stderr, "Failed to open encrypted log file\n");
        free(log_ciphertext);
        exit(EXIT_FAILURE);
    }

    fwrite(iv, 1, AES_IVLEN, encrypted_log_file); // Write IV to the beginning of the output file
    fwrite(log_ciphertext, 1, log_ciphertext_len, encrypted_log_file);
    fclose(encrypted_log_file);
    free(log_ciphertext);

    if (!move_file_to_parent(log_output_path)) {
        fprintf(stderr, "Failed to move encrypted log file\n");
        exit(EXIT_FAILURE);
    }

    if (remove(LOG_FILE) != 0) {
        fprintf(stderr, "Failed to delete log file\n");
    }

    printf("Process completed successfully.\n");
    return 0;
}

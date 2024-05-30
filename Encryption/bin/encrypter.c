#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <time.h>
#include <stdbool.h>

#define DATA_DIR "../Data"
#define LOG_FILE "./log.order"
#define KEYPAIR_DIR "../Data"
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

bool is_unique(char **existing_strings, size_t count, const char *new_string) {
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(existing_strings[i], new_string) == 0) {
            return false;
        }
    }
    return true;
}

// Function to check if the path exists and if it is a file
int is_valid_file(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0; // Path does not exist
    }
    if (S_ISREG(statbuf.st_mode)) {
        return 1; // Path is a file
    }
    return 0; // Path is not a file
}

// Function to read a key from a file
int read_key(const char *key_file, uint8_t *key, size_t key_len) {
    FILE *file = fopen(key_file, "rb");
    if (!file) {
        return 0; // Failed to open key file
    }
    fread(key, 1, key_len, file);
    fclose(file);
    return 1;
}

// Function to encrypt a file using AES-256
int encrypt_file(const char *input_path, const char *output_path, uint8_t *key, uint8_t *iv) {
    FILE *input_file = fopen(input_path, "rb");
    FILE *output_file = fopen(output_path, "wb");
    if (!input_file || !output_file) {
        return 0; // Failed to open files
    }

    fwrite(iv, 1, AES_IVLEN, output_file); // Write IV to the beginning of the output file

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return 0; // Failed to create cipher context
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0; // Failed to initialize encryption
    }

    uint8_t buffer[4096];
    uint8_t ciphertext[4096 + EVP_CIPHER_block_size(EVP_aes_256_cbc())];
    int len, ciphertext_len;

    while ((len = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
        if (EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len, buffer, len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return 0; // Failed to encrypt data
        }
        fwrite(ciphertext, 1, ciphertext_len, output_file);
    }

    if (EVP_EncryptFinal_ex(ctx, ciphertext, &ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0; // Failed to finalize encryption
    }
    fwrite(ciphertext, 1, ciphertext_len, output_file);

    EVP_CIPHER_CTX_free(ctx);
    fclose(input_file);
    fclose(output_file);

    return 1;
}

// Function to handle logging
void handle_logging(const char *keypair_name, const char *x, int clear_log) {
    FILE *log_file = fopen(LOG_FILE, clear_log ? "w" : "a");
    if (!log_file) {
        fprintf(stderr, "Failed to open log file\n");
        return;
    }
    fprintf(log_file, "../Data/%s: %s: File\n", keypair_name, x);
    fclose(log_file);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <path_to_data> <key_index> [clear_log]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); // Seed the random number generator

    const char *data_path = argv[1];
    int key_index = atoi(argv[2]) - 1;
    int clear_log = (argc == 4);

    if (!is_valid_file(data_path)) {
        fprintf(stderr, "Invalid path: %s\n", data_path);
        exit(EXIT_FAILURE);
    }

    char key_file[512];
    snprintf(key_file, sizeof(key_file), "%s/secret_key_%d.bin", KEYPAIR_DIR, key_index);
    if (!is_valid_file(key_file)) {
        fprintf(stderr, "Invalid key index: %d\n", key_index + 1);
        exit(EXIT_FAILURE);
    }

    uint8_t key[AES_KEYLEN];
    if (!read_key(key_file, key, sizeof(key))) {
        fprintf(stderr, "Failed to read key file: %s\n", key_file);
        exit(EXIT_FAILURE);
    }

    uint8_t iv[AES_IVLEN];
    if (!RAND_bytes(iv, sizeof(iv))) {
        fprintf(stderr, "Failed to generate IV\n");
        exit(EXIT_FAILURE);
    }

    char encrypted_name[10];
    generate_random_string(encrypted_name, 9);

    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s.enc", DATA_DIR, encrypted_name);

    if (!encrypt_file(data_path, output_path, key, iv)) {
        fprintf(stderr, "Failed to encrypt file: %s\n", data_path);
        exit(EXIT_FAILURE);
    }

    if (clear_log) {
        char response;
        printf("Are you sure you want to clear the log file? (Y/n): ");
        scanf(" %c", &response);
        if (response != 'Y' && response != 'y') {
            clear_log = 0;
        }
    }

    handle_logging(strrchr(key_file, '/') + 1, encrypted_name, clear_log);

    printf("Encrypted %s to %s\n", data_path, output_path);
    return 0;
}

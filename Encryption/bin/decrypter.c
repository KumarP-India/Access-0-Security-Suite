#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <unistd.h>
#include <termios.h>

#define DECRYPTED_DIR "../Decrypted"

// Function to check if the path exists and is a file
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

// Function to get a hidden input from the user (for key entry)
void get_hidden_input(char *input, size_t len) {
    struct termios oldt, newt;
    printf("Enter key: ");
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fgets(input, len, stdin);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    input[strcspn(input, "\n")] = '\0';
    printf("\n");
}

// Function to decrypt a file using AES-256
int decrypt_file(const char *input_path, const char *output_path, uint8_t *key, uint8_t *iv) {
    FILE *input_file = fopen(input_path, "rb");
    FILE *output_file = fopen(output_path, "wb");
    if (!input_file || !output_file) {
        return 0; // Failed to open files
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return 0; // Failed to create cipher context
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0; // Failed to initialize decryption
    }

    uint8_t buffer[4096];
    uint8_t plaintext[4096 + EVP_CIPHER_block_size(EVP_aes_256_cbc())];
    int len, plaintext_len;

    while ((len = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
        if (EVP_DecryptUpdate(ctx, plaintext, &plaintext_len, buffer, len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return 0; // Failed to decrypt data
        }
        fwrite(plaintext, 1, plaintext_len, output_file);
    }

    if (EVP_DecryptFinal_ex(ctx, plaintext, &plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0; // Failed to finalize decryption
    }
    fwrite(plaintext, 1, plaintext_len, output_file);

    EVP_CIPHER_CTX_free(ctx);
    fclose(input_file);
    fclose(output_file);

    return 1;
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <path_to_file> <mode> <new_name> [key_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];
    int mode = atoi(argv[2]);
    const char *new_name = argv[3];
    const char *key_file = (argc == 5) ? argv[4] : NULL;

    if (!is_valid_file(file_path)) {
        fprintf(stderr, "Invalid file path: %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    uint8_t key[32];
    uint8_t iv[AES_BLOCK_SIZE] = {0}; // Assuming the IV is all zeros (or adapt as needed)

    if (mode == 0) {
        if (!key_file || !is_valid_file(key_file)) {
            fprintf(stderr, "Invalid key file path: %s\n", key_file);
            exit(EXIT_FAILURE);
        }
        if (!read_key(key_file, key, sizeof(key))) {
            fprintf(stderr, "Failed to read key file: %s\n", key_file);
            exit(EXIT_FAILURE);
        }
    } else if (mode == 1) {
        char key_input[64];
        get_hidden_input(key_input, sizeof(key_input));
        strncpy((char *)key, key_input, sizeof(key));
    } else {
        fprintf(stderr, "Invalid mode: %d\n", mode);
        exit(EXIT_FAILURE);
    }

    mkdir(DECRYPTED_DIR, 0700);

    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s", DECRYPTED_DIR, new_name);

    if (!decrypt_file(file_path, output_path, key, iv)) {
        fprintf(stderr, "Failed to decrypt file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    printf("Decrypted %s to %s\n", file_path, output_path);
    return 0;
}

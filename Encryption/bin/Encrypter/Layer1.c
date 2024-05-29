/*
* @Author: Prabhas Kumar
* @Assistant: OpenAI ChatGPT 4o

* @Created: May 29'24
* @Updated: None

* @Project: Access-0-Security-Suite 's Encryption Version 1
* @File: Layer1 [C script]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SECRET_KEY_PATH "../../keypair1_secret_key.bin"
#define DATA_FOLDER_PATH "../../../Data/decrypted"
#define OUTPUT_FOLDER_PATH "../../../Data/"

// Function to encrypt a single file
void encrypt_file(const char *input_file, const char *output_file, const uint8_t *key) {
    FILE *in = fopen(input_file, "rb");
    if (in == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    uint8_t *file_content = malloc(file_size);
    fread(file_content, 1, file_size, in);
    fclose(in);

    uint8_t iv[EVP_MAX_IV_LENGTH];
    if (!RAND_bytes(iv, sizeof(iv))) {
        fprintf(stderr, "RAND_bytes failed\n");
        exit(EXIT_FAILURE);
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "EVP_CIPHER_CTX_new failed\n");
        exit(EXIT_FAILURE);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "EVP_EncryptInit_ex failed\n");
        exit(EXIT_FAILURE);
    }

    int outlen;
    uint8_t *encrypted_content = malloc(file_size + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    if (EVP_EncryptUpdate(ctx, encrypted_content, &outlen, file_content, file_size) != 1) {
        fprintf(stderr, "EVP_EncryptUpdate failed\n");
        exit(EXIT_FAILURE);
    }
    int ciphertext_len = outlen;
    if (EVP_EncryptFinal_ex(ctx, encrypted_content + outlen, &outlen) != 1) {
        fprintf(stderr, "EVP_EncryptFinal_ex failed\n");
        exit(EXIT_FAILURE);
    }
    ciphertext_len += outlen;

    EVP_CIPHER_CTX_free(ctx);
    free(file_content);

    FILE *out = fopen(output_file, "wb");
    if (out == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fwrite(iv, 1, sizeof(iv), out);
    fwrite(encrypted_content, 1, ciphertext_len, out);
    fclose(out);

    free(encrypted_content);
}

int main() {
    // Load the secret key
    FILE *key_file = fopen(SECRET_KEY_PATH, "rb");
    if (key_file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    uint8_t key[32]; // AES-256 key size is 32 bytes
    fread(key, 1, sizeof(key), key_file);
    fclose(key_file);

    // Ensure the output directory exists
    DIR *output_dir = opendir(OUTPUT_FOLDER_PATH);
    if (output_dir == NULL) {
        if (mkdir(OUTPUT_FOLDER_PATH, 0777) != 0) {
            perror("mkdir");
            return EXIT_FAILURE;
        }
    } else {
        closedir(output_dir);
    }

    // Open the data directory
    DIR *dir = opendir(DATA_FOLDER_PATH);
    if (dir == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char input_file[256];
            snprintf(input_file, sizeof(input_file), "%s%s", DATA_FOLDER_PATH, entry->d_name);

            char output_file[256];
            snprintf(output_file, sizeof(output_file), "%s%s.enc", OUTPUT_FOLDER_PATH, entry->d_name);

            encrypt_file(input_file, output_file, key);
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

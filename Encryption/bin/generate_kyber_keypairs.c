#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <oqs/oqs.h>

// Define the fixed output directory
#define OUTPUT_DIR "../../Data/"

// Function to check if the path exists and is a directory
int is_valid_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0; // Path does not exist
    }
    if (S_ISDIR(statbuf.st_mode)) {
        return 1; // Path is a directory
    }
    return 0; // Path is not a directory
}

// Function to generate Kyber key pairs and save to the specified path
void generate_kyber_keypairs(int n, const char *path) {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_512);
    if (kem == NULL) {
        fprintf(stderr, "Failed to initialize Kyber KEM\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        uint8_t public_key[OQS_KEM_kyber_512_length_public_key];
        uint8_t secret_key[OQS_KEM_kyber_512_length_secret_key];

        OQS_STATUS status = OQS_KEM_keypair(kem, public_key, secret_key);
        if (status != OQS_SUCCESS) {
            fprintf(stderr, "Failed to generate key pair %d\n", i);
            continue;
        }

        char pubkey_filename[256];
        char seckey_filename[256];
        snprintf(pubkey_filename, sizeof(pubkey_filename), "%s/public_key_%d.bin", path, i);
        snprintf(seckey_filename, sizeof(seckey_filename), "%s/secret_key_%d.bin", path, i);

        FILE *pubkey_file = fopen(pubkey_filename, "wb");
        FILE *seckey_file = fopen(seckey_filename, "wb");

        if (pubkey_file && seckey_file) {
            fwrite(public_key, 1, sizeof(public_key), pubkey_file);
            fwrite(secret_key, 1, sizeof(secret_key), seckey_file);
            fclose(pubkey_file);
            fclose(seckey_file);
        } else {
            fprintf(stderr, "Failed to open files for key pair %d\n", i);
        }
    }

    OQS_KEM_free(kem);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_keypairs>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    const char *path = OUTPUT_DIR;

    if (!is_valid_directory(path)) {
        fprintf(stderr, "Invalid directory path: %s\n", path);
        exit(EXIT_FAILURE);
    }

    generate_kyber_keypairs(n, path);

    printf("Generated %d Kyber key pairs in directory: %s\n", n, path);
    return 0;
}

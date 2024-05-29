/*
* @Author: Prabhas Kumar
* @Assistant: OpenAI ChatGPT 4.0

* @Created: May 29'24
* @Updated: None

* @Project: Access-0-Security-Suite 's Encryption Version 1
* @File: Key_Generator [C script]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>

void save_key(const char *filename, const uint8_t *key, size_t length) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fwrite(key, 1, length, file);
    fclose(file);
}

void save_passphrase(const char *filename, const char *passphrase) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fwrite(passphrase, 1, strlen(passphrase), file);
    fclose(file);
}

void generate_and_save_keys(const char *prefix, OQS_KEM *kem) {
    uint8_t pk[kem->length_public_key];
    uint8_t sk[kem->length_secret_key];

    if (OQS_KEM_keypair(kem, pk, sk) != OQS_SUCCESS) {
        fprintf(stderr, "OQS_KEM_keypair failed\n");
        exit(EXIT_FAILURE);
    }

    char pk_filename[256];
    char sk_filename[256];
    snprintf(pk_filename, sizeof(pk_filename), "../../%s_public_key.bin", prefix);
    snprintf(sk_filename, sizeof(sk_filename), "../../%s_secret_key.bin", prefix);

    save_key(pk_filename, pk, kem->length_public_key);
    save_key(sk_filename, sk, kem->length_secret_key);
}

int main() {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_1024);
    if (kem == NULL) {
        fprintf(stderr, "OQS_KEM_new failed\n");
        exit(EXIT_FAILURE);
    }

    generate_and_save_keys("keypair1", kem);
    generate_and_save_keys("keypair2", kem);
    generate_and_save_keys("keypair3", kem);

    // Get custom passphrase from user
    char passphrase[65]; // 64 characters + null terminator
    printf("Enter a 64-character passphrase: ");
    if (fgets(passphrase, sizeof(passphrase), stdin) == NULL) {
        fprintf(stderr, "Error reading passphrase\n");
        exit(EXIT_FAILURE);
    }

    // Remove newline character if present
    size_t len = strlen(passphrase);
    if (len > 0 && passphrase[len - 1] == '\n') {
        passphrase[len - 1] = '\0';
    }

    if (strlen(passphrase) != 64) {
        fprintf(stderr, "Passphrase must be exactly 64 characters long\n");
        exit(EXIT_FAILURE);
    }

    save_passphrase("../../custom_passphrase_key.bin", passphrase);

    OQS_KEM_free(kem);
    return 0;
}

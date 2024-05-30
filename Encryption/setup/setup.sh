#!/bin/bash

# @Author: Prabhas Kumar
# @Assistant: OpenAI ChatGPT 4o
#
# @Created: May 29'24
# @Updated: None
#
# @Project: Access-0-Security-Suite 's Encryption Version 1
# @File: setup [shell script]

# Install Open Quantum Safe (OQS) provides a C to get implementation of Kyber and gcc to compile c
brew install liboqs gcc

# We will now compile all of the c files. Note as mentioned in ../README.md make changes to variables below if need arises

# Define variables for Homebrew paths
HOMEBREW_PREFIX=$(brew --prefix)
INCLUDE_PATH="$HOMEBREW_PREFIX/include"
LIB_PATH="$HOMEBREW_PREFIX/lib"

gcc ../bin/generate_kyber_keypairs.c -o ../New_Keys -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/encrypter.c -o ../Encrypter -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/decrypter.c -o ../Decrypter -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/finaliser.c -o ../Cleaner -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto

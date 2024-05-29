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

mkdir ../programs/
mkdir ../programs/Decrypter
gcc ../bin/Decrypter/Layer1.c -o ../programs/Decrypter/Layer1 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Decrypter/Layer2.c -o ../programs/Decrypter/Layer2 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Decrypter/Layer3-5.c -o ../programs/Decrypter/Layer3-5 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Decrypter/Layer6.c -o ../programs/Decrypter/Layer6 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Decrypter/Layer7.c -o ../programs/Decrypter/Layer7 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Decrypter/main.c -o ../programs/Decrypter/main -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto

mkdir ../programs/Encrypter
gcc ../bin/Encrypter/Layer1.c -o ../programs/Encrypter/Layer1 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Encrypter/Layer2.c -o ../programs/Encrypter/Layer2 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Encrypter/Layer3-5.c -o ../programs/Encrypter/Layer3-5 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Encrypter/Layer6.c -o ../programs/Encrypter/Layer6 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Encrypter/Layer7.c -o ../programs/Encrypter/Layer7 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Encrypter/main.c -o ../programs/Encrypter/main -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto

mkdir ../programs/Key_and_Order
gcc ../bin/Key_and_Order/Key_Generator.c -o ../programs/Key_and_Order/Layer1 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto
gcc ../bin/Key_and_Order/Order_Generator.c -o ../programs/Key_and_Order/Layer2 -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto

#!/bin/bash

# @Author: Prabhas Kumar
# @Assistant: OpenAI ChatGPT 4o
#
# @Created: May 29'24
# @Updated: None
#
# @Project: Access-0-Security-Suite 's Encryption Version 1
# @File: setup [shell script]

# Define colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'



# Check if the system is MacOS
if [[ "$(uname)" == "Darwin" ]]; then
    echo -e "${GREEN}[Passed] ${NC}MacOS system detected."

    # Check the architecture
    ARCHITECTURE=$(uname -m)
    
    if [[ "$ARCHITECTURE" == "arm64" ]]; then
        echo -e "${GREEN}[Passed] ${NC}Apple M chip (ARM architecture) detected."

        # Install Open Quantum Safe (OQS) that provides a C to get implementation of Kyber and gcc to compile c
        brew install liboqs gcc &&

        echo -e "${GREEN}[FINISHED] ${NC}Required libraries downloaded and installed sucessfully."
        

        # We will now compile all of the c files. Note as mentioned in ../../../README.md make changes to variables below if need arises.
    
        # Define variables for Homebrew paths
        HOMEBREW_PREFIX=$(brew --prefix)
        INCLUDE_PATH="$HOMEBREW_PREFIX/include"
        LIB_PATH="$HOMEBREW_PREFIX/lib"
        
        gcc "../../../bin/MacOS (M chips)/generate_kyber_keypairs.c" -o "../../../New_Keys" -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto &&
        gcc "../../../bin/MacOS (M chips)/encrypter.c" -o "../../../Encrypter" -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto &&
        gcc "../../../bin/MacOS (M chips)/decrypter.c" -o "../../../Decrypter" -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto &&
        gcc "../../../bin/MacOS (M chips)/finalizer.c" -o "../../../Cleaner" -I"$INCLUDE_PATH" -L"$LIB_PATH" -loqs -lcrypto &&

        echo -e "${GREEN}[FINISHED] ${NC}Programs compiled; setup is finished."

    elif [[ "$ARCHITECTURE" == "x86_64" ]]; then
        echo -e "${RED}[FAILED] ${NC}Intel x86 processor detected."
    else
        echo -e "${RED}[Warning] ${NC}Unknown architecture: $ARCHITECTURE"
    fi
else
    echo -e "${RED}[Failed] ${NC}MacOS system not detected."
fi

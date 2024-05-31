# Encryption Directory

Welcome! This directory handles data encryption. It is designed to encrypt any data as long as a local path is provided. The encryption uses AES 256-CBC, with the option to either use a custom password or generate key pairs using Kyber 512. Note that encryption can only be done one file at a time. For decryption, the program accepts either a custom password or a key path.

## Key Points

- **Encryption Method**: AES 256-CBC
- **Key Options**: Custom password or Kyber 512 key pairs
- **Single File Encryption**: Only one file can be encrypted at a time
- **Log File**: Maintains `log.order` for tracking encryption order when using Kyber keys
- **File Renaming**: Encrypted files are renamed to avoid recognition

## Setup

Every OS has a setup file. Navigate to your OS and its configuration subfolder, then run the setup file in the terminal or PowerShell.

```bash
cd Encryption/"[your OS and its configuration subfolder]"
./setup[.sh or .ps1]
```

**Example for MacOS with M chips**:

```bash
cd "./Encryption/setup/MacOS/Apple M chips (ARM)"
./setup.sh
```

## Usage

After setup, you will have four programs in the `./Encryption` directory.

### New_Keys

Generate new Kyber Keys by providing an integer input as a terminal argument.

```bash
cd ./Encryption
./New_Keys [number_of_keypairs]
```

**Example**: Generate 17 keys

```bash
cd ./Encryption
./New_Keys 17
```

Keys will be generated in the `./Data` directory.

### Encrypter

Encrypt a single file at a time using Kyber-generated keys. Provide the key index and an optional argument to clear the `log.order`. Furthermore it also hides the file anme and extention by using 9 charector random string.

```bash
cd ./Encryption
./encrypter [path_to_data] [key_index] [clear_log (optional, any string detected triggers this)]
```

**Note**: Key generator indexes from `0`, but for human-friendly counting, use `1` for index `0`.

**Example 1**: Encrypt `data.png` outside the repo with key index 4 (actual index 3)

```bash
cd ./Encryption
./encrypter ../../data.png 4
```

**Example 2**: Encrypt `data.png` and clear `log.order`

```bash
cd ./Encryption
./encrypter ../../data.png 4 z
<type y when asked>
```

### Decrypter

Decrypt files with two modes: `[0]` for `.bin` keys and `[1]` for custom passwords. Provide the necessary file paths and keys.

```bash
cd ./Encryption
./decrypter [path_to_file] [mode, 0 or 1] [before_encryting_name] [key_file (optional)]
```

**Example 1**: Decrypt `QWERWEQFP.enc` with a custom password

```bash
cd ./Encryption
./decrypter ../../QWERWEQFP.enc 1 data.pages
<Type password when asked>
```

**Example 2**: Decrypt `IIYYEEFFQ.enc` with `public_key_1.bin`

```bash
cd ./Encryption
./decrypter ../../IIYYEEFFQ.enc 0 data.pdf ../Data/public_key_1.bin
```

### Cleaner

Ensure no unencrypted key and `log.order` file is left.

```bash
cd ./Encryption
./Cleaner
```

## How I Will Use It

My encryption scheme involves multiple layers and nested encryption. I will use the `Encrypter` on files and their encrypted versions, generating thrice the number of keys for enhanced security.

## Planned Updates

1. Support for more OS and architectures.
2. Automatic encryption of all files in folders and subfolders.
3. There is small chance that randome generated file name would already be used earlier for some other data. Handle this by testing and generating new ones till finding unique name.

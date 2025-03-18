# dcrypt

## Description


### **Explanation of the dcrypt Header Structure:**

When dcrypt encrypts a file, it modifies the original content by adding a **custom header**. This header stores metadata needed to decrypt the file and manage encrypted content.

| **Offset**  | **Value**                              | **Description**                           |
|-------------|---------------------------------------|-------------------------------------------|
| **0x0000**  | `"TODCRYPT"`                           | Magic value/signature to identify encrypted files. |
| **0x0008**  | **Length of RSA encrypted data**       | Size (in bytes) of the encrypted AES key. Typically **256 bytes** for a 2048-bit RSA key. |
| **0x000C**  | **RSA encrypted AES file encryption key** | AES-128 key (16 bytes) is encrypted using **RSA-2048** and stored here. |
| **0x010C**  | **Initialization Vector used during AES encryption**     | Random IV is used during AES encryption and stored here. |
| **0x0110**  | **Original file size**                 | The size of the original unencrypted file. Helps verify successful decryption. |
| **0x0118**  | **Encrypted file contents (AES-128 CBC)** | The actual **AES-128 CBC** encrypted content of the original file.

### Data encryption

* Our keygen function generates a random encryption key of a specified width using a given character set. It seeds the random number generator with the current time, and selects random characters from the character set to build the key.

---

## Commands

```sh
# Encryption
make && ./dcrypt

# Decryption
make && ./dcrypt -r <KEY>
```

---

## Testing

```sh
# Cleans, create test folder with test files, compiles, then runs the program
make run

# Creates test files to the test folder
make quine

# Or if you want 30 files for each given extension
make quine n=30 ext="txt vob cpp crt"
```

## TODO
- encrypted folders list
- iv to header
-
# **dcrypt**

## **Description**

dcrypt is a file encryption/decryption tool that adds a custom header to encrypted files, storing essential metadata for decryption and management of the encrypted content.

### **Explanation of the dcrypt Header Structure:**

When dcrypt encrypts a file, it modifies the original content by adding a **custom header**. This header stores metadata needed to decrypt the file and manage encrypted content.

| **Offset**  | **Value**                              | **Description**                           |
|-------------|---------------------------------------|-------------------------------------------|
| **0x0000**  | `"TODCRYPT"`                           | Magic value/signature to identify encrypted files. |
| **0x0008**  | **Initialization Vector used during AES encryption**     | Random IV is used during AES encryption and stored here in plaintext. |
| **0x001e**  | **Original file size**                 | The size of the original unencrypted file. Helps verify successful decryption. |
| **0x0026**  | **Encrypted file contents (AES-128 CBC)** | The actual **AES-128 CBC** encrypted content of the original file.

### **Data encryption**
dcrypt performs AES-128 CBC encryption in order to encrypt the data.

#### **Chaining**
In CBC mode, each plaintext block is XORed with the previous ciphertext block before being encrypted.<br />
This means that the encryption of each block depends on the previous block's ciphertext, which enhances security.

#### **IV (Initialization Vector)**
The IV is a random value used to ensure that identical plaintext blocks produce different ciphertexts.<br />
This prevents patterns from appearing in the encrypted data.
<br /><br />
- Without IV (= NULL IV) the process will be faster, but less secure.
- Identical plaintexts will produce identical ciphertexts.
- dcrypt uses the keygen mentionned below to create a unique IV.
- The used IV is then saved on the custom header and is retrieved during decryption.

#### **Padding**

When using OpenSSL's EVP functions for encryption, the default padding scheme used is PKCS#7 padding.<br />
If the plaintext data is not a multiple of the block size (16 bytes for AES), PKCS#7 padding adds bytes to the end of the plaintext.

#### **Keygen for 128bit keys**
Our keygen function generates a random encryption key of a specified width using a given character set. It seeds the random number generator with the current time, and selects random characters from the character set to build the key.

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
- cross platform
# **dcrypt**

## **Description**

dcrypt is a file encryption/decryption tool that adds a custom header to encrypted files, storing essential metadata for decryption and management of the encrypted content.

### **Features**

- dcrypt only works in the specified directories inside `dcrypt.h`:
```
# define DC_TARGET_PATHS { "/home/user/folder1", /home/user/folder2" } // No '/' at the end
```
- The program will only act on files whose extensions have been affected by Wannacry. The list of the handled extensions can be found in `extensions.c`:

```c
const char *handled_extensions[] = {
    "der", "pfx", "key", "crt", "csr", "p12", "pem", "odt", "ott", "sxw", "stw", "uot",
    "3ds", "max", "3dm", "ods", "ots", "sxc", "stc", "dif", "slk", "wb2", "odp", "otp",
    "sxd", "std", "uop", "odg", "otg", "sxm", "mml", "lay", "lay6", "asc", "sqlite3",
    "sqlitedb", "sql", "accdb", "mdb", "db", "dbf", "odb", "frm", "myd", "myi", "ibd",
    "mdf", "ldf", "sln", "suo", "cs", "c", "cpp", "pas", "h", "asm", "js", "cmd", "bat",
    "ps1", "vbs", "vb", "pl", "dip", "dch", "sch", "brd", "jsp", "php", "asp", "rb",
    "java", "jar", "class", "sh", "mp3", "wav", "swf", "fla", "wmv", "mpg", "vob",
    "mpeg", "asf", "avi", "mov", "mp4", "3gp", "mkv", "3g2", "flv", "wma", "mid",
    "m3u", "m4u", "djvu", "svg", "ai", "psd", "nef", "tiff", "tif", "cgm", "raw",
    "gif", "png", "bmp", "jpg", "jpeg", "vcd", "iso", "backup", "zip", "rar", "7z",
    "gz", "tgz", "tar", "bak", "tbk", "bz2", "PAQ", "ARC", "aes", "gpg", "vmx", 
    "vmdk", "vdi", "sldm", "sldx", "sti", "sxi", "602", "hwp", "snt", "onetoc2", 
    "dwg", "pdf", "wk1", "wks", "123", "rtf", "csv", "txt", "vsdx", "vsd", "edb", 
    "eml", "msg", "ost", "pst", "potm", "potx", "ppam", "ppsx", "ppsm", "pps", 
    "pot", "pptm", "pptx", "ppt", "xltm", "xltx", "xlc", "xlm", "xlt", "xlw", 
    "xlsb", "xlsm", "xlsx", "xls", "dotx", "dotm", "dot", "docm", "docb", "docx", 
    "doc"
};
```

 - The program will encrypt the contents of the files in this folder using a key.
- Files are encrypted with the AES-128 CBC algorithm.
- The program renames all the files in the mentioned folders adding the ".dcrypt" extension.
- If they already have this extension, they will not be renamed.
- The key with which the files are encrypted have to be 16 characters long.
- The program will do the reverse operation using the encryption key in order to restore the files to their original state.

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
# Compilation
make

# Usage:
./dcrypt [-h|-v|-s|-k <KEY>|-r <KEY>] 

 Options:
  -v, --version          Show version information
  -s, --silent           Run in silent mode (non-verbose)
  -k, --key <KEY>        Provide an encryption key
  -r, --reverse <KEY>    Decrypt using the provided decryption key
  -h, --help             Show this help message and exit
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
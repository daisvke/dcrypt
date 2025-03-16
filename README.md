# Stockholm

## Description

## WannaCry

### **Explanation of WannaCry Header Structure:**

When WannaCry encrypts a file, it modifies the original content by adding a **custom header**. This header stores metadata needed to decrypt the file and manage encrypted content.

| **Offset**  | **Value**                              | **Description**                           |
|-------------|---------------------------------------|-------------------------------------------|
| **0x0000**  | `"WANACRY!"`                           | Magic value/signature to identify encrypted files. |
| **0x0008**  | **Length of RSA encrypted data**       | Size (in bytes) of the encrypted AES key. Typically **256 bytes** for a 2048-bit RSA key. |
| **0x000C**  | **RSA encrypted AES file encryption key** | AES-128 key (16 bytes) is encrypted using **RSA-2048** and stored here. |
| **0x010C**  | **File type internal to WannaCry**     | Likely an identifier of the original file type (e.g., EXE, DOCX, etc.). Used during decryption to restore file format. |
| **0x0110**  | **Original file size**                 | The size of the original unencrypted file. Helps verify successful decryption. |
| **0x0118**  | **Encrypted file contents (AES-128 CBC)** | The actual **AES-128 CBC** encrypted content of the original file. |

---

### **How Does WannaCry Encrypt Files?**
1. **Generate AES Key**: A random **AES-128** key is created for each file.
   
2. **Encrypt File with AES-128 CBC**: 
   - The original file is encrypted using **AES-128 CBC**.
   - A unique **IV (Initialization Vector)** is used for each file to prevent pattern analysis.

3. **Encrypt AES Key with RSA-2048**: 
   - The AES key is encrypted using a **2048-bit RSA public key** (from the ransomware operator).
   - Only the attacker’s **private key** can decrypt it.

4. **Add WannaCry Header**: 
   - **Magic Signature**: `"WANACRY!"` marks files as encrypted.
   - **RSA Encrypted Key**: Stores the encrypted AES key.
   - **Original File Metadata**: Includes file type and original size.

5. **Overwrite Original File**: The original file is replaced by the **WannaCry-encrypted** version.


### Data encryption
* Our keygen function generates a random encryption key of a specified width using a given character set. It seeds the random number generator with the current time, and selects random characters from the character set to build the key.
* We've enhanced our XOR-based encryption algorithm by incorporating an additive cipher. The additive cipher, also known as a Caesar cipher, shifts each character of the plaintext by a fixed amount (the key) before performing the XOR operation. This additional step adds another layer of complexity to the encryption process.

## Commands


## Useful Links
[WannaCry Malware Profile](https://cloud.google.com/blog/topics/threat-intelligence/wannacry-malware-profile)
[WannaCry Handled File Extensions](https://gist.githubusercontent.com/xpn/facb5692980c14df272b16a4ee6a29d5/raw/57232fe6b3014c5562f878dd8aab74af3d74c24f/wannacry_file_extensions.txt)

## TODO
- make -w option: if not active, key not saved in header, given as arg after -r
-  If they already have this extension, they will not be renamed.
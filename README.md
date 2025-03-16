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

### Payload
By default, the payload is only a string that will be displayed before the execution of the target program.
However we have added a 'virus' mode that in fact is a quine binary executed by the shellcode; a piece of code that replicates itself on the current directory.

## Commands
```
// Make and run the packer with the default options. Then, run the packed binary. All with valgrind
make run

// Run the packer with verbose mode, padding injection and virus mode on, then run the binary. without the -v, the program will not display anything neither on the standard output nor on the error output.
./famine /bin/ls -v -i=p -s=v

// Useful commands
readelf -l [filename]	// Check program headers of the file
readelf -S [filename]	// Check section headers

hexdump -C [filename]	// Check the file in hex format

vimdiff [filename 1] [filename 2] // Check the difference between two files

// Extract the .text section from code.o
objcopy --dump-section .text=code-raw code.o

// Print the loaded file content in hex format at address 0x401040
gdb ./woody
run (or r)
x/16xw 0x401040

// Add breakpoint at relative address 11ad if base address is 0x4011ad
b *0x4011ad

// Produce a trace trap that stops the execution at the position (useful when debugging)
int3
```

## Notes
```
    lodsb             ; Load the next byte of the message into AL
    xor al, key       ; XOR the byte with the key
    stosb             ; Store the encrypted byte back into memory
    loop encrypt_loop ; Repeat for the entire message



    lodsb:
        lodsb stands for "load byte from source into AL."
        It is used to load a byte from the memory location pointed to by the ESI register into the AL register.
        After loading the byte, the ESI register is automatically incremented or decremented based on the direction flag (DF).
        This instruction is commonly used in string operations to process bytes sequentially.

    stosb:
        stosb stands for "store byte from AL into destination."
        It is used to store the byte in the AL register into the memory location pointed to by the EDI register.
        After storing the byte, the EDI register is automatically incremented or decremented based on the direction flag (DF).
        This instruction is also commonly used in string operations to write bytes sequentially to memory.
The Direction Flag (DF) in x86 assembly language is a flag in the FLAGS register that controls the direction of string operations. The DF flag can be set or cleared using the std (set direction) and cld (clear direction) instructions, respectively. (default=0)

```

## Useful Links
https://packetstormsecurity.com/files/12327/elf-pv.txt.html<br />
https://grugq.github.io/docs/phrack-58-05.txt<br />
https://wh0rd.org/books/linkers-and-loaders/linkers_and_loaders.pdf<br />
https://hackademics.fr/forum/hacking-connaissances-avanc%C3%A9es/reversing/autres-aq/3096-reversing-tutorials-level-2-le-format-elf
https://book.jorianwoltjer.com/binary-exploitation/shellcode

## TODO
- make -w option: if not active, key not saved in header, given as arg after -r
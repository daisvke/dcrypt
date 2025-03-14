# famine

## Description

This project is about coding a packer and an unpacker shellcode for ELF (Executable and Linkable Format) 64-bit binary files. The packer will pack all the files in the current directory.
<br /><br />
"Packers" are tools whose task consists of compressing executable programs (.exe, .dll,.ocx,...) and/or encrypting them. <br />
During the execution of a packer, a program passing through that packer is loaded in memory, compressed and encrypted. Then, during execution of the packed program, through the shellcode unpacker the packer will have injected, it will be decompressed, decrypted, and finally be executed.
<br /><br />
The existence of such programs is related to the fact that antivirus programs generally analyse programs when they are loaded in memory, before they are executed. <br />
Thus, encryption and compression of a packer allow to bypass this behavior by obfuscating the content of an executable until its execution.

## Technical aspects
### Packers and unpackers
```
Packer                                        Unpacker
+-------------------------+                   +-------------------------+
|                         |                   |                         |
|    Input Binary         |                   |   Encrypted/Compressed  |
|                         |                   |       Binary            |
+----------+--------------+                   +-----------+-------------+
           |                                              |
           |                                              |
           |                                              |
           v                                              v
+----------+--------------+                   +-----------+-------------+
|                         |                   |                         |
|    Encrypted/Compressed |                   |  Decrypted/Decompressed |
|        Binary           |                   |         Binary          |
|                         |                   |                         |
+-------------------------+                   +-------------------------+
```
Two distinct elements are involved: the packer and the unpacker.

The packer is a separate program that is used to encrypt or compress the target binary. It takes the original binary as input, applies specific encryption or compression techniques, and generates an encrypted or compressed binary as output. The packer is responsible for creating a specific format or structure for the encrypted/compressed binary, which may include additional information such as decryption routines or metadata.

Next, the unpacker is included inside the generated file during the build process. It is a special code or routine that is loaded and executed when the generated file is started. Its role is to decrypt the encrypted binary and/or decompress the compressed binary to restore it to its original form before execution.

### The 64-bit ELF file structure

Below is a textual representation of a binary file with its ELF header (64-bit) and other regions, along with their byte sizes and corresponding structures:

```
+----------------------------------------------------+
|                    ELF Header                      |
|            (64 bytes, ELF64_Ehdr structure)        |
+----------------------------------------------------+
|                Program Header Table                |
|   (Variable size, contains ELF64_Phdr structures)  |
|                 contains info about:               |
|                     Segment 1                      |
|                     Segment 2...                   |
|                  ...Segemnt n                      |
+----------------------------------------------------+
|                   Text Section                     |
|               (Variable size and data)             |
+----------------------------------------------------+
|                   Data Section                     |
|               (Variable size and data)             |
+----------------------------------------------------+
|                 Symbol Table Section               |
|   (Variable size, contains ELF64_Sym structures)   |
+----------------------------------------------------+
|                 String Table Section               |
|          (Variable size, contains strings)         |      
|       Contains a single string in the format:      |
|              "name1\0name2\0name3\0"               |
+----------------------------------------------------+
|                Section Header Table (optional)     |
|   (Variable size, contains ELF64_Shdr structures)  |
+----------------------------------------------------+
```

Here's a brief explanation of each component:

- ELF Header: The ELF header contains essential information about the file, including the ELF identification, file type, architecture, entry point, program header table offset, section header table offset, and more.

- Program Header Table: This table describes the segments or sections in the binary file, such as the code segment, data segment, and dynamic linking information. Each entry follows the ELF64_Phdr structure and provides details like the segment type, offset, virtual address, file size, and memory size. The size of the program header table can vary depending on the number of entries.<br />
Details about program headers and segments can be displayed by the command: readelf -l.

- Section Header Table: This table contains information about each section in the binary file, such as the name, type, flags, offset, size, and more. Each entry follows the ELF64_Shdr structure. The section header table size can vary based on the number of sections.

- Data Section: This section holds initialized data used by the program. It can have a variable size depending on the data present in the binary.

- Text Section: This section contains the executable code of the program. Its size varies based on the code written.

- Symbol Table Section: This section stores information about symbols defined or referenced by the program, such as function and variable names, their addresses, and other attributes. Each entry in the symbol table follows the ELF64_Sym structure. The symbol table size can vary depending on the number of symbols.

- String Table Section: This section stores the names of various symbols, section names, and other string data referenced by the binary. It has a variable size and contains substrings grouped in a single string.

Please note that the actual sizes and structures may vary depending on the specific binary format and the contents of the file. The representation provided here gives a general overview of the components typically found in a binary file with a 64-bit ELF header.<br />
Thus, an EIP-independent program would be preferable for our case.<br />
When software or code is EIP-independent, it means that it does not rely on specific memory addresses or offsets relative to the EIP register, making it more reliable across different environments.
<br /><br />
Please check man ELF for more details.

### ELF Infection

* In the context of packers and unpackers, the stub and the unpacker are closely related but not necessarily synonymous terms.
<br /><br />
The stub refers to a small piece of code that is inserted into the packed binary. Its purpose is to perform the initial processing and setup necessary for the unpacker to execute. The stub typically contains minimal functionality and is responsible for locating and executing the unpacker code.
<br />
The unpacker, on the other hand, is the actual code responsible for unpacking or decrypting the packed portions of the binary. It extracts the original, unpacked code and data, restoring the binary to its original state.

* When inserting the stub or the additional code into an ELF file, there are a few common approaches to consider:

    1. At the End of the file/a segment/a section: One approach is to append the stub code at the end of a block of code. This can be done by extending the file size and writing the new code at the appended location, then shifting all the elements behind.

    2. In Padding Areas: ELF files often have padding areas between sections or segments. These padding areas are typically filled with zeroes. We can utilize these unused spaces to insert the stub code. However, these padding areas may not always be present or of sufficient size, depending on the specific ELF file structure.

    3. Reserved Sections: If the ELF file contains reserved or unused sections, we can repurpose one of these sections to hold the stub code. In that case we have to make sure that the selected section does not interfere with the original functionality of the ELF file or any dependencies.

Our program uses the second approach by default. However, in case the padding size is inferior to the stub size, then it uses the first approach, inserting the stub after the executable segment.

* The program will act on the paths given in the `FA_TARGET_PATHS` global variable. By default, these are `/tmp/test` and `/tmp/test2` folders
* Only one infection per binary is possible.
* Infections will be on the binaries of the operating system that have a 64 bits architecture

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

# Stockholm

## Instructions

### Compilation
```
make
```

### Running
```sh
# Encryption
#
# This generates a random encryption key that will be displayed
# on the terminal if the silence mode (-s/--silence) is off
./stockholm

# Decryption
./stockholm -r <KEY>

# With a custom key
./stockholm -k <KEY>		# Encryption
./stockholm -r <KEY>		# Decryption
```

### Testing
```sh
# Cleans, create test folder with test files, compiles, then runs the program
make run

# Creates test files to the test folder
make quine

# Or if you want 30 files for each given extension
make quine n=30 ext="txt vob cpp crt"
```
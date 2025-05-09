# ▗▄▄▄  ▗▄▄▖▗▄▄▖▗▖  ▗▖▗▄▄▖▗▄▄▄▖
# ▐▌  █▐▌   ▐▌ ▐▌▝▚▞▘ ▐▌ ▐▌ █  
# ▐▌  █▐▌   ▐▛▀▚▖ ▐▌  ▐▛▀▘  █  
# ▐▙▄▄▀▝▚▄▄▖▐▌ ▐▌ ▐▌  ▐▌    █  
                                     

ifeq ($(TARGET), win)
	NAME				= dcrypt.exe
else
	NAME				= dcrypt
endif

# ****************************
#       ANSI ESCAPE CODES
# ****************************

# ANSI escape codes for stylized output
RESET 				= \033[0m
GREEN				= \033[32m
YELLOW				= \033[33m
RED					= \033[31m

# Logs levels
INFO 				= $(YELLOW)[INFO]$(RESET)
ERROR				= $(RED)[ERROR]$(RESET)
DONE				= $(GREEN)[DONE]$(RESET)


# ****************************
#       BUILD COMMANDS
# ****************************

TARGET				?= unix

# Compiler selection based on TARGET
ifeq ($(TARGET), win)  # Windows
	TEMP_FOLDER1	= D:\Documents\infection
    CC				= x86_64-w64-mingw32-gcc
    CFLAGS			= -Wall -Wextra -O2
else
	TEMP_FOLDER1	= /home/alien/infection
    CC				= clang
    CFLAGS			= -Wall -Wextra -O2
	SSLFLAGS		= -lcrypto -lssl
endif


# ****************************
#       BUILD FILES
# ****************************

# Source files
SRCS_DIR			= srcs/
SRCS_FILES			= $(notdir $(wildcard $(SRCS_DIR)*.c))

# Include files
INCS_DIR			= incs/
INCS_FILES			= $(notdir $(wildcard $(INCS_DIR)*.h))
INCS 				= $(addprefix $(INCS_DIR), $(INCS_FILES))

# Obj files
OBJS_DIR			= objs/
OBJS				= $(addprefix $(OBJS_DIR), $(SRCS_FILES:.c=.o))

# Test files
QUINE_NAME			= bacteria

# Path for a test file to copy to the folders above
SOURCE_TEST_NAME1 	= sample.txt
SOURCE_TEST_FILE1 	= resources/$(SOURCE_TEST_NAME1)

# Path for the files inside the target temporary folder
TARGET_TEST_FILE1 	= $(TEMP_FOLDER1)/$(QUINE_NAME).txt
TARGET_TEST_FILE2 	= $(TEMP_FOLDER1)/$(SOURCE_TEST_NAME1)


# ****************************
#       BUILDING
# ****************************

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCS)
ifeq ($(TARGET), win)
	@PowerShell -Command " \
		if (-Not (Test-Path -Path '$(OBJS_DIR)')) \
			{ New-Item -Path '$(OBJS_DIR)' -ItemType Directory }"
else
	mkdir -p $(OBJS_DIR)
endif
	$(CC) -I$(INCS_DIR) -c $(CFLAGS) $< -o $@

all:
ifeq ($(TARGET), win)
	$(MAKE) win
else
	$(MAKE) unix
endif

.PHONY: unix
unix: $(OBJS)
	@echo "$(INFO) Building for Linux"
	$(CC) $(CFLAGS) $(SSLFLAGS) $(OBJS) -o $(NAME)

.PHONY: win
win: $(OBJS)
	@echo "Building for Windows"
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)


# ****************************
#       TESTING
# ****************************

.PHONY: setup
# Setup for the tests
setup:
ifeq ($(TARGET), win)
	@PowerShell -Command " \
		if (-Not (Test-Path -Path '$(TEMP_FOLDER1)')) \
			{ New-Item -Path '$(TEMP_FOLDER1)' -ItemType Directory }; \
			Copy-Item -Path '$(SOURCE_TEST_FILE1)' -Destination '$(TEMP_FOLDER1)'"
else
	mkdir -p $(TEMP_FOLDER1)
	cp $(SOURCE_TEST_FILE1) $(TEMP_FOLDER1)
endif

# Set default values for arguments (can be given from command line)
i	?= 10
ext	?= txt vob pdf crt gif

.PHONY: quine
# Spread quines on the test folder to get a huge amount of test files
quine:
	mkdir -p $(TEMP_FOLDER1)
	rm -rf $(QUINE_NAME)/
	git clone https://github.com/daisvke/$(QUINE_NAME)
	@make -C $(QUINE_NAME)/C/ collection i=$(i) ext="$(ext)"
	cp $(QUINE_NAME)/C/$(QUINE_NAME)_* $(TEMP_FOLDER1)/
	rm -rf $(QUINE_NAME)/

.PHONY: run
# A quick test that copies/creates test files on the temporary folder,
#	compiles the project and runs the binary with valgrind
run: setup quine
	valgrind ./$(NAME)
	@echo "\n$(DONE) After a successful encryption you can reverse the process with:"
	@echo "\t\t./$(NAME) -r <KEY>"
	@echo "\tand check if the files are back to their original states."

.PHONY: debug
debug: CFLAGS += -g3 -DDEBUG
debug: $(NAME)


# ****************************
#       CLEANING
# ****************************

.PHONY: clean fclean re


# The use of try-catch let us skip errors
clean:
ifeq ($(TARGET), win)
	@PowerShell -Command " \
		try { \
			Remove-Item -Path '$(OBJS_DIR)', '$(TEMP_FOLDER1)' -Recurse -Force -ErrorAction Stop \
		} catch { \
			Write-Host 'Error removing folder: ' $$_.Exception.Message \
		}"
else
	rm -rf $(OBJS_DIR)
endif

fclean: clean
ifeq ($(TARGET), win)
	@PowerShell -Command " \
		try { \
			Remove-Item -Path '$(NAME)' -Force -ErrorAction Stop \
		} catch { \
			Write-Host 'Error removing folder: ' $$_.Exception.Message \
		}"
else
	rm -f $(NAME)
endif

re: fclean setup
ifeq ($(TARGET), win)
	$(MAKE) win
else
	$(MAKE) unix
endif

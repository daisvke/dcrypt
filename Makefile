# ▗▄▄▄  ▗▄▄▖▗▄▄▖▗▖  ▗▖▗▄▄▖▗▄▄▄▖
# ▐▌  █▐▌   ▐▌ ▐▌▝▚▞▘ ▐▌ ▐▌ █  
# ▐▌  █▐▌   ▐▛▀▚▖ ▐▌  ▐▛▀▘  █  
# ▐▙▄▄▀▝▚▄▄▖▐▌ ▐▌ ▐▌  ▐▌    █  
                                     


NAME				= dcrypt

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

WCC					= x86_64-w64-mingw32-gcc
WLIBS				= -lws2_32
WCFLAGS				= -Wall -Wextra $(LIBS) -O2

CC					= clang
CFLAGS				= -Wall -Wextra -O2
SSLFLAGS			= -lcrypto -lssl


# ****************************
#       BUILD FILES
# ****************************

# Source files

SRCS_DIR			= srcs/
SRCS_FILES			= $(notdir $(wildcard $(SRCS_DIR)*.c))

WSRCS_DIR			= srcs/
WSRCS_FILES			= $(notdir $(wildcard $(WSRCS_DIR)*.c))

# Include files

INCS_DIR			= incs/
INCS_FILES			= $(notdir $(wildcard $(INCS_DIR)*.h))
INCS 				= $(addprefix $(INCS_DIR), $(INCS_FILES))

# Obj files

OBJS_DIR			= objs/
OBJS				= $(addprefix $(OBJS_DIR), $(SRCS_FILES:.c=.o))

WOBJS_DIR			= objs/
WOBJS				= $(addprefix $(WOBJS_DIR), $(WSRCS_FILES:.c=.o))
# Test files

# Temporary folders where binaries are copied to
TEMP_FOLDER1		= ~/infection
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

.PHONY: all

all: $(NAME)

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCS)
	mkdir -p $(OBJS_DIR)
	$(CC) -I$(INCS_DIR) -c $(CFLAGS) $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(SSLFLAGS) $(OBJS) -o $(NAME)

$(WOBJS_DIR)%.o: $(WSRCS_DIR)%.c $(INCS)
	@if not exist $(OBJS_DIR) ( mkdir -p $(OBJS_DIR) )
	$(WCC) -I$(INCS_DIR) -c $(WCFLAGS) $< -o $@

windows: $(WOBJS)
	$(WCC) $(WCFLAGS) $(WOBJS) -o $(NAME)


# ****************************
#       TESTING
# ****************************

.PHONY: setup

# Setup for the tests
setup:
	mkdir -p $(TEMP_FOLDER1)
	cp $(SOURCE_TEST_FILE1) $(TEMP_FOLDER1)

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
run: re setup quine
	valgrind ./$(NAME)
	@echo "\n$(DONE) Now you can reverse the encryption with:"
	@echo "\t\t./$(NAME) -r <KEY>"
	@echo "\tand check if the files are back to their original states."

.PHONY: debug

debug: CFLAGS += -g3 -DDEBUG
debug: $(NAME)


# ****************************
#       CLEANING
# ****************************

.PHONY: clean fclean re

clean:
	rm -rf $(OBJS_DIR) $(ASM_OBJS_DIR) $(TEMP_FOLDER1)

fclean: clean
	rm -rf $(NAME)

re: fclean all

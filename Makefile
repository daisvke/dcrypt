# **************************************************************************** #
#       TITLE                                                                  #
# **************************************************************************** #
NAME				= stockholm

# **************************************************************************** #
#       COMMANDS                                                               #
# **************************************************************************** #
CC					= clang
ASM					= nasm
ASOBJS_FLAGS		= -f elf64
ASFLAGS				= -f bin

# **************************************************************************** #
#       FLAGS                                                                  #
# **************************************************************************** #
CFLAGS				= -Wall -Wextra
SSLFLAGS			= -lcrypto -lssl

# **************************************************************************** #
#       SOURCES                                                                #
# **************************************************************************** #
SRCS_DIR			= srcs/
SRCS_FILES			= $(notdir $(wildcard $(SRCS_DIR)*.c))

ASM_SRCS_DIR		= srcs/
ASM_SRCS_FILES		= $(notdir $(wildcard $(ASM_SRCS_DIR)*.s))
ASM_SRCS			= $(addprefix $(ASM_SRCS_DIR), $(ASM_SRCS_FILES))

STUB_SRCS_DIR		= $(SRCS_DIR)unpacker/
STUB_SRCS_FILES		= $(notdir $(wildcard $(STUB_SRCS_DIR)*.s))

# **************************************************************************** #
#       INCLUDES                                                               #
# **************************************************************************** #
INCS 				= stockholm.h $(STUB_HDRS)

# **************************************************************************** #
#       OBJ                                                                    #
# **************************************************************************** #
OBJS_DIR			= objs/
OBJS				= $(addprefix $(OBJS_DIR), $(SRCS_FILES:.c=.o))

ASM_OBJS_DIR		= $(OBJS_DIR)
ASM_OBJS			= $(addprefix $(ASM_OBJS_DIR), $(ASM_SRCS_FILES:.s=.o))

# **************************************************************************** #
#       TEST FILES                                                             #
# **************************************************************************** #
# Temporary folders where binaries are copied to
TEMP_FOLDER1		= ~/infection
# Path for binaries to copy to the folders above (targets)
SOURCE_TEST_FILE1 	= resources/sample.txt
SOURCE_TEST_FILE2 	=
SOURCE_TEST_FILE3 	=
SOURCE_TEST_FILE4 	=
# Path for the binaries inside the target temporary folder
TARGET_TEST_FILE1 	= $(TEMP_FOLDER1)/sample.txt
TARGET_TEST_FILE2 	=

# **************************************************************************** #
#       RULES                                                                  #
# **************************************************************************** #

.PHONY: all run debug clean fclean re debug

all: $(NAME)

$(ASM_OBJS_DIR)%.o: $(ASM_SRCS_DIR)%.s
	mkdir -p $(ASM_OBJS_DIR)
	$(ASM) $(ASOBJS_FLAGS) $< -o $@

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCS)
	mkdir -p $(OBJS_DIR)
	$(CC) -I. -c $(CFLAGS) $< -o $@

$(NAME): $(OBJS) $(ASM_OBJS)
	$(CC) $(CFLAGS) $(SSLFLAGS) $(OBJS) $(ASM_OBJS) -o $(NAME)

clean_asm:
	rm -rf asm

# Setup for the tests
setup:
	rm -f $(TARGET_TEST_FILE1) $(TARGET_TEST_FILE2)
	mkdir -p $(TEMP_FOLDER1)
	cp $(SOURCE_TEST_FILE1) $(TEMP_FOLDER1)

# Spread quines on the test folder to get a huge amount of test files
quine:
	rm -rf bacteria/
	git clone git@github.com:daisvke/bacteria.git
	$(MAKE) run_one_quine n=3 ext=txt
	$(MAKE) run_one_quine n=3 ext=pdf
	$(MAKE) run_one_quine n=3 ext=iso
	$(MAKE) run_one_quine n=3 ext=xls
	$(MAKE) run_one_quine n=3 ext=cpp
	rm -rf bacteria/

run_one_quine:
	@if [ -z "$(n)" ] || [ -z "$(ext)" ]; then \
		echo "Error: Arguments 'n' and 'ext' must be provided."; \
		exit 1; \
	fi
	find bacteria/C/ -type f -exec sed -i 's/i = [0-9]\+/i = $(n)/g; s/EXT ".*/EXT ".$(ext)"/g' {} \;
	make -C bacteria/C/
	cp bacteria/C/bacteria $(TEMP_FOLDER1)/
	cd $(TEMP_FOLDER1) && ./bacteria

# A quick test that copies the target binaries to the temporary folder
#	and runs the compilation + packer + packed file wirh valgrind
run: re setup
	valgrind ./$(NAME)
	@echo "\n-----------------------------test"
	$(TARGET_TEST_FILE1)
	@echo "-----------------------------"
	$(TARGET_TEST_FILE2)
	@echo "\n-----------------------------test2"
	$(TARGET_TEST_FILE3)
	@echo "-----------------------------"
	$(TARGET_TEST_FILE4)

debug: CFLAGS += -g3 -DDEBUG
debug: $(NAME)

clean:
	rm -rf $(OBJS_DIR) $(ASM_OBJS_DIR) $(TARGET_TEST_FILE1) $(TARGET_TEST_FILE2) $(TARGET_TEST_FILE3) $(TARGET_TEST_FILE4)

fclean: clean
	rm -rf $(NAME) $(TEMP_FOLDER1)

re: fclean all

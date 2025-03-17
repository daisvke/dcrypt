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
#       BUILDING                                                               #
# **************************************************************************** #

.PHONY: all

all: $(NAME)

$(ASM_OBJS_DIR)%.o: $(ASM_SRCS_DIR)%.s
	mkdir -p $(ASM_OBJS_DIR)
	$(ASM) $(ASOBJS_FLAGS) $< -o $@

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCS)
	mkdir -p $(OBJS_DIR)
	$(CC) -I. -c $(CFLAGS) $< -o $@

$(NAME): $(OBJS) $(ASM_OBJS)
	$(CC) $(CFLAGS) $(SSLFLAGS) $(OBJS) $(ASM_OBJS) -o $(NAME)


# **************************************************************************** #
#       TESTING                                                                #
# **************************************************************************** #

.PHONY: setup

# Setup for the tests
setup:
	mkdir -p $(TEMP_FOLDER1)
	rm -f $(TARGET_TEST_FILE1) $(TARGET_TEST_FILE2)
	mkdir -p $(TEMP_FOLDER1)
	cp $(SOURCE_TEST_FILE1) $(TEMP_FOLDER1)

# Spread quines on the test folder to get a huge amount of test files
QUINE_NAME = bacteria

# 
i	?= 10
ext	?= txt vob pdf crt gif

.PHONY: quine

quine:
	mkdir -p $(TEMP_FOLDER1)
	rm -rf $(QUINE_NAME)/
	git clone https://github.com/daisvke/$(QUINE_NAME)
	@make -C $(QUINE_NAME)/C/ collection i=$(i) ext="$(ext)"
	cp $(QUINE_NAME)/C/$(QUINE_NAME)_* $(TEMP_FOLDER1)/
	rm -rf $(QUINE_NAME)/

.PHONY: run

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

.PHONY: debug

debug: CFLAGS += -g3 -DDEBUG
debug: $(NAME)


# **************************************************************************** #
#       CLEANING                                                               #
# **************************************************************************** #

.PHONY: clean fclean re

clean:
	rm -rf $(OBJS_DIR) $(ASM_OBJS_DIR)
	rm -f $(TARGET_TEST_FILE1) $(TARGET_TEST_FILE2) $(TARGET_TEST_FILE3) $(TARGET_TEST_FILE4)
	rm -f $(TEMP_FOLDER1)/$(QUINE_NAME)_*

fclean: clean
	rm -rf $(NAME) $(TEMP_FOLDER1)

re: fclean all

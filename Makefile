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
TEMP_FOLDER1		= /tmp/test
TEMP_FOLDER2		= /tmp/test2
# Path for binaries to copy to the folders above (targets)
SOURCE_BIN1 		= resources/64bit-sample
SOURCE_BIN2 		= resources/32bit-sample
SOURCE_BIN3 		= /bin/ls
SOURCE_BIN4 		= /bin/df
# Path for the binaries inside the target temporary folder
TARGET_BIN1 		= $(TEMP_FOLDER1)/64bit-sample
TARGET_BIN2 		= $(TEMP_FOLDER1)/32bit-sample
TARGET_BIN3 		= $(TEMP_FOLDER1)/ls
TARGET_BIN4 		= $(TEMP_FOLDER1)/df

# **************************************************************************** #
#       RULES                                                                  #
# **************************************************************************** #

.PHONY: all run debug clean fclean re debug

all: $(STUB_OBJS) $(NAME)

$(ASM_OBJS_DIR)%.o: $(ASM_SRCS_DIR)%.s
	mkdir -p $(ASM_OBJS_DIR)
	$(ASM) $(ASOBJS_FLAGS) $< -o $@

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCS)
	mkdir -p $(OBJS_DIR)
	$(CC) -I. -c $(CFLAGS) $< -o $@

$(NAME): $(OBJS) $(ASM_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(ASM_OBJS) -o $(NAME)

clean_asm:
	rm -rf asm

# Setup for the tests
setup:
	mkdir -p $(TEMP_FOLDER1)
	mkdir -p $(TEMP_FOLDER2)
	cp $(SOURCE_BIN1) $(TEMP_FOLDER1)
	cp $(SOURCE_BIN2) $(TEMP_FOLDER1)
	cp $(SOURCE_BIN3) $(TEMP_FOLDER1)
	cp $(SOURCE_BIN4) $(TEMP_FOLDER1)

# A quick test that copies the target binaries to the temporary folder
#	and runs the compilation + packer + packed file wirh valgrind
run: re setup
	valgrind ./$(NAME)
	@echo "\n-----------------------------test"
	$(TARGET_BIN1)
	@echo "-----------------------------"
	$(TARGET_BIN2)
	@echo "\n-----------------------------test2"
	$(TARGET_BIN3)
	@echo "-----------------------------"
	$(TARGET_BIN4)


debug: CFLAGS += -g3 -DDEBUG
debug: $(NAME)

clean:
	rm -rf $(OBJS_DIR) $(ASM_OBJS_DIR) $(TARGET_BIN1) $(TARGET_BIN2) $(TARGET_BIN3) $(TARGET_BIN4)

fclean: clean
	rm -f $(NAME)

re: fclean all

NAME = visualizer

# DEBUG_FLAGS = -fsanitize=address -g
DEBUG_FLAGS = -g
FLAGS = $(DEBUG_FLAGS)
INCLUDES = -I incl

LIB_DIR = libs/
# STATIC_LIB = -l SDL2 -l SDL2_image -l SDL2_ttf -lm -ldl -lpthread
STATIC_LIB = -l SDL3

SRCS_DIR       = srcs/

BIN_DIR = bin/


SRCS_NAMES =			\
	input				\
	main 				\
	project				\
	sdl_init			\


SRCS = $(addprefix $(SRCS_DIR), $(addsuffix .cpp, $(SRCS_NAMES)))

OBJS = $(addprefix $(BIN_DIR), $(SRCS:.c=.o))

all: $(NAME)
# all:
# 	echo $(SRCS)
$(NAME): $(BIN_DIR) $(OBJS)
	g++ $(FLAGS) $(INCLUDES) -o $(NAME) $(OBJS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)%.o: %.c
	mkdir -p $(BIN_DIR)$(dir $<)
	g++ $(FLAGS) $(INCLUDES) -c $< -o $@

run: re clean
	./$(NAME)

clean:
	rm -rf $(BIN_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
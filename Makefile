CFLAGS=-Werror -Wextra -Wall -g3 -Wno-missing-field-initializers
INCLUDES=-I./include/
NAME=lair


all: bin $(NAME)

clean:
	rm -f *.o
	rm -f $(NAME)

%.o: ./src/%.c
	$(CC) $(CFLAGS) $(LIB_INCLUDES) $(INCLUDES) -c $<

bin: $(NAME)
$(NAME): main.o gc.o vector.o error.o lair_std.o eval.o map.o parse.o lair.o
	$(CC) $(CLAGS) $(LIB_INCLUDES) $(INCLUDES) -o $(NAME) $^ -lm $(LIBS)

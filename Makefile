CFLAGS=-Werror -Wextra -Wall -g3 -Wno-missing-field-initializers
INCLUDES=-I./include/
NAME=lair
OBJ=gc.o vector.o error.o lair_std.o eval.o map.o parse.o lair.o


all: unit_test bin

clean:
	rm -f *.o
	rm -f $(NAME)

%.o: ./src/%.c
	$(CC) $(CFLAGS) $(LIB_INCLUDES) $(INCLUDES) -c $<

bin: $(NAME)
$(NAME): main.o $(OBJ)
	$(CC) $(CLAGS) $(LIB_INCLUDES) $(INCLUDES) -o $(NAME) $^ -lm $(LIBS)

unit_test: unit_test.o $(OBJ)

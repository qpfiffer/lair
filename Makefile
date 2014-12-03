CFLAGS=-Werror -Wall -O2 -g3
INCLUDES=-I./include/
CC=clang
NAME=lair


all: bin $(NAME)

clean:
	rm -f *.o
	rm -f $(NAME)

%.o: ./src/%.c
	$(CC) $(CFLAGS) $(LIB_INCLUDES) $(INCLUDES) -c $<

bin: $(NAME)
$(NAME): main.o eval.o map.o parse.o lair.o
	$(CC) $(CLAGS) $(LIB_INCLUDES) $(INCLUDES) -o $(NAME) $^ -lm $(LIBS)

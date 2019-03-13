![Original picture David Greenwell](./lair.jpg?raw=true)

[![Build Status](https://drone.io/github.com/qpfiffer/lair/status.png)](https://drone.io/github.com/qpfiffer/lair/latest)

> "It is a dark world where all you have are functions, and all you can do is
> apply them to each other." - Anonymous

# Done status

- [x] Can print stuff
- [x] Arguments
- [x] Functions-as-arguments
- [x] If statements ('?')
- [x] Loops (In the form of recursion?)
- [ ] Arrays/Dictionaries
- [ ] Nested Functions

This is an experimental [Den](https://github.com/MuteLang/DenLang) implementation. It
intends to support all of the same syntax with some extra goodies.

### Goals

This is the first language I've ever written an interpreter for, so it is as
much a learning process as anything else.

* Strong, Dynamic types (Erlang style)
* String internment
* Persistent data structures
* Immutability
* Functions as first-class citizens
* Low-memory footprint

### Installation

1. `make`
2. Thats it. You should now have a `lair` binary.

#### Debugging

Edit the `Makefile` to turn on the `DEBUG` flag, then rebuild:

```Makefile
    CFLAGS=-Werror -Wextra -Wall -g3 -Wno-missing-field-initializers -DDEBUG
```

```Bash
    make clean
    make
```

Now you'll get extra fancy, obtuse output:

```Bash
$ ./lair t/functions_all_the_way_down.den 
FUNCTION a FUNCTION_ARG {
    RETURN CALL ATOM ATOM 
}
FUNCTION b FUNCTION_ARG 
    RETURN CALL ATOM ATOM 
}
FUNCTION c FUNCTION_ARG 
    RETURN CALL ATOM 
}
FUNCTION d 
    RETURN STRING 
}
CALL ATOM CALL ATOM ATOM EOF 
---
This is the real test.
```

### Usage

Currently there is no REPL for testing, so for now usage is limited to loading
a file, parsing it, executing it and then doing whatever the program wanted via
STDOUT/STDIN.

    ./lair t/basic.den

### Documentation

Documentation is done with [Doxygen](http://www.stack.nl/~dimitri/doxygen/).
Note that this is for Lair itself, not for Den.

To build docs:

    doxygen lair.conf

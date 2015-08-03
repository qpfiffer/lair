> "It is a dark world where all you have are functions, and all you can do is
> apply them to each other." - Anonymous

# Done status

- [x] Can print stuff
- [x] Arguments
- [x] Functions-as-arguments
- [ ] If statements ('?')
- [ ] Loops
- [ ] Arrays
- [ ] Nested Functions

This is an experimental [Den](http://wiki.xxiivv.com/den) implementation. It
intends to support all of the same syntax with some extra goodies.

### Goals

This is the first language I've ever written an interpreter for, so it is as
much a learning process as anything else.

* Strong, Dynamic types (Erlang style)
* String internment
* Persistent data structures
* Immutability
* Functions as first-class citizens

### Installation

1. `make`
2. Thats it. You should now have a `lair` binary.

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

# Axios
Axios is an extremely minimalistic programming language with two core operations. Its name comes from the Greek origin of the word "axiom", and this language is my attempt to explore the fundamental building blocks that can form any theoretical concept.

Axios was created with these goals in mind:
 * To have as few core operations as possible (two, excluding the I/O operations), such that...
   * Each operation is simple to explain
   * Each operation performs the same function(s) in all circumstances
 * To be Turing complete
 * To make writing invalid code impossible; all sequences of characters are valid in Axios and cannot yield a syntax or runtime error (except in instances of excessive memory use)

The language is explained in much greater detail in the guide. Below is a fairly brief summary of how Axios works.

Axios operates on a list of cells that grow over time. Each cell has two possible values, zero or one (not to be confused with the 0 and 1 operators). In this implimentation, cells are stored as bits, but other data types like booleans can also serve this purpose. There is also a pointer located along the list.

In addition, code is always divided into states (similar to a Turing machine). At the beginning of each state, the cell at which the pointer is located is always changed.

The two core operators are 0 and 1, and 2 and 3 are used for output and input respectively. Note that Axios does not require these operators be Western Arabic numerals, as other numeral systems can be used to write programs. In my view, for a programming language with as few operations as Axios, there are no advantages to relying solely on one writing system. It is more important that as many people as possible can type valid code.

In short, the operators work as follows:

Program start: Initializes list containing one cell set to zero (note that the first state must immediately change this cell to one)

1: Divides the code into states. Thus, a different state can be found to the right and to the left of each 1 (even if it contains no operators). There is also the termination state, which lies after the last state visible in code. 

0: Performs two functions
* The presence of a 0 operator indicates the pointer remains where it is. The absence of a 0 operator indicates the pointer moves forward. If the pointer has to move when it is already at the end of the list, a new cell is added to the end, and the pointer returns to the beginning.
* The number of 0 operators indicates the next state. By default (i.e. no 0 operators), the next state is the next one written in code. Otherwise:
  * If the current cell is one (after changing it), each 0 operator indicates that the next state moves back one. So one 0 repeats the current state written in code, two 0s goes to the state immediately before the current state as written in code, etc. 
  * If the current cell is zero (after changing it), continues as normal to the next state written in code.

There are also two I/O operators:

2: Outputs the value of the current cell as a bit (after changing the value of the cell, before moving the pointer, and before changing states, if applicable). Each time 21 bits are output, the program displays a character if the output forms a valid UTF-32 encoding (in little-endian order). Attempting to output 21 ones also resets the list used for the 3 operator. (There is no valid character that can be output by 21 ones.)

3: Changes cells according to the individual bits contained in UTF-32 characters input by the user, which are stored in a list in little-endian order. The 3 operator only asks for user input when it uses all the bits in the list (21 bits per UTF-32 character; other bits stored in UTF-32 characters must always be zeroes and are therefore ignored). It is also possible to empty the contents of this list by attempting to output 21 ones. This operation occurs at the beginning of a state, and its presence within a state means the current cell will not be changed except according to user input.

While I know Axios can simulate all the operations of Turing complete languages, I am still working on a formal proof. At this time, my informal reasoning is that many operations of a Turing machine can be simulated trivially, namely changing the current cell, moving to the right, and shifting between states. Moving to the left is more complicated, but it involves leaving a marker of where the pointer was previously, and looping back around to it.

Numeral systems supported by this Axios implementation currently include Western Arabic, Eastern Arabic, Devanagari, Persian, Bengali, Tamil, Thai, Lao, Tibetan, Burmese, Khmer, and fullwidth numerals.

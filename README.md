# Axios
Axios is an extremely minimalistic programming language with two core operations. Its name comes from the Greek origin of the word "axiom", and this language is my attempt to explore the fundamental building blocks that can form any theoretical concept. A nice feature of Axios is it is impossible to write invalid code that could throw an error (unless too much storage space is used). To understand how all this works, it is important to know several fundamental features.

First, Axios operates on a list of cells that grow over time. Each cell has two possible values, zero or one (not to be confused with the 0 and 1 operators). In this implimentation, cells are stored as bits, but other data types like booleans can also serve this purpose. There is also a pointer located along the list.

Second, code is always divided into states (similar to a Turing machine). At the beginning of each state, the cell at which the pointer is located is changed.


From here, the operators work as follows:

Program start: Initialize list containing one cell set to zero (note that the first state must immediately change this cell to one)

1: Divides the code into states. Thus, a different state can be found to the right and to the left of each 1 (even if it looks empty). There is also the termination state, which lies after the last state visible in code. 

0: Performs two functions (in order)
* Each 0 operator changes the decision of whether to move the pointer forward along the list. In other words, an odd number of 0s in a state indicates moving forward, and an even number of 0s does not move the pointer. If the pointer is already located at the end of the list, a new cell initilized to zero is appended to the end of the list, and the pointer returns to the beginning.
* Decides which state to go to next.
  * If the value at the current cell is set to zero (after moving the pointer, if applicable), continues on to the beginning of the next listed state in the code (the state immediately following the 1). Users should know that, if the current cell is zero for all states, the program must terminate.
  * If the value at the current cell is set to one, each 0 operator performs a "zig-zag" along the code. So the absence of a 0 means continuing to the next state as usual, one 0 returns to the beginning of the current state, two 0s goes to the beginning of the state after the next, three 0s goes to the beginning of the state before the current one, etc. Note that this zig-zag pattern can "loop" around the entire code, but keep in mind there is a termination state that is not immediately visible. Going to the state immediately after the last encoded state or immediately before the first will terminate the program.


There are also two I/O operators:

2: Outputs the value of the current cell as a bit (after moving the pointer and before changing states, if applicable). Each time thirty-two bits are output, the program displays a character if the output forms a valid UTF-32 encoding (in little-endian order). Outputting 32 ones also resets the list used for the 3 operator. Otherwise, nothing happens.

3: Changes cells according to the individual bits contained in UTF-32 characters input by the user, which are stored in a list in little-endian order. The 3 operator only asks for user input when it uses all the bits in the list. It is also possible to reset this list by attempting to output 32 ones.

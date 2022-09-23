/*
MIT License

Copyright (c) 2022 Maxine Dobbs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>

/*
Contents:

Input Queue
  *  struct node
  *  remove_front()
  *  clear_queue()
  *  add_inputs(): adds elements to the queue from user input

display_output()

run_program()

Parse Code
  *  identify_three_byte_operator()
  *  identify_two_byte_operator()
  *  read_program()

Input Code
  *  run_code_from_file()
  *  run_shell()

main(): runs menu
*/


// Input Queue

// Node struct used to implement queue for inputting bits
struct node {
    unsigned long input_utf_32;
    struct node *next;
} *front, *rear;

// Removes the first node of the input queue
void remove_front() {
    struct node *nd;
    nd = front;
    front = front->next;
    free(nd);
}

// Empties the queue, except for the terminal element
void clear_queue() {
    while (front->input_utf_32 != 0xFFFFFFFF)
        remove_front();
}

// Adds all nodes to the input queue based on the input from input_bits()
unsigned long add_inputs(unsigned char *ptr, unsigned long input_queue_size) {
    while (*ptr != '\n') {
        if (*ptr > 0xEF && *ptr < 0xF8) {
            if (*(ptr+1) > 0x7F && *(ptr+1) < 0xC0 && *(ptr+2) > 0x7F && *(ptr+2) < 0xC0 && *(ptr+3) > 0x7F && *(ptr+3) < 0xC0) {
                rear->input_utf_32 = (*ptr - 0xF0) * 0x40000 + (*(ptr+1) - 0x80) * 0x1000 + (*(ptr+2) - 0x80) * 0x40 + *(ptr+3) - 0x80;
                ptr += 4;
            } else {
                fprintf(stderr, "\nInvalid UTF-8 encoding input, cannot convert to UTF-32\n");
                return 0xFFFFFFFF;
            }
        } else if (*ptr > 0xDF && *ptr < 0xF0) {
            if (*(ptr+1) > 0x7F && *(ptr+1) < 0xC0 && *(ptr+2) > 0x7F && *(ptr+2) < 0xC0) {
                rear->input_utf_32 = (*ptr - 0xE0) * 0x1000 + (*(ptr+1) - 0x80) * 0x40 + *(ptr+2) - 0x80;
                ptr += 3;
            } else {
                fprintf(stderr, "\nInvalid UTF-8 encoding input, cannot convert to UTF-32\n");
                return 0xFFFFFFFF;
            }
        } else if (*ptr > 0xBF && *ptr < 0xE0) {
            if (*(ptr+1) > 0x7F && *(ptr+1) < 0xC0) {
                rear->input_utf_32 = (*ptr - 0xC0) * 0x40 + *(ptr+1) - 0x80;
                ptr += 2;
            } else {
                fprintf(stderr, "\nInvalid UTF-8 encoding input, cannot convert to UTF-32\n");
                return 0xFFFFFFFF;
            }
        } else if (*ptr < 0x80)
            rear->input_utf_32 = *(ptr++);
        else {
            fprintf(stderr, "\nInvalid UTF-8 encoding input, cannot convert to UTF-32\n");
            return 0xFFFFFFFF;
        }

        rear->next = (struct node*) malloc(sizeof(struct node));
        rear = rear->next;
        if (rear == NULL) {
            fprintf(stderr, "\nFailed to allocate memory\n");
            return 0xFFFFFFFF;
        }

        if (++input_queue_size > 1048576) {
            fprintf(stderr, "\nInput is too long, exceeding limits on memory\n");
            return 0xFFFFFFFF;
        }
    }
    rear->input_utf_32 = '\n';
    rear->next = (struct node*) malloc(sizeof(struct node));
    rear = rear->next;
    rear->input_utf_32 = 0xFFFFFFFF;
    return input_queue_size + 1;
}



// Converts UTF-32 encoding to UTF-8 and prints it to the screen
void display_output(unsigned long output_utf_32) {
    unsigned char output_string[5];
    output_string[4] = 0;
    if (output_utf_32 > 0xFFFF) {
        output_string[3] = 0x80 + output_utf_32 % 0x40;
        output_string[2] = 0x80 + output_utf_32 % 0x1000 / 0x40;
        output_string[1] = 0x80 + output_utf_32 % 0x40000 / 0x1000;
        output_string[0] = 0xF0 + output_utf_32 / 0x40000;
    } else if (output_utf_32 > 0x7FF) {
        output_string[3] = 0;
        output_string[2] = 0x80 + output_utf_32 % 0x40;
        output_string[1] = 0x80 + output_utf_32 % 0x1000 / 0x40;
        output_string[0] = 0xE0 + output_utf_32 / 0x1000;
    } else if (output_utf_32 > 0x7F) {
        output_string[3] = 0;
        output_string[2] = 0;
        output_string[1] = 0x80 + output_utf_32 % 0x40;
        output_string[0] = 0xC0 + output_utf_32 / 0x40;
    } else {
        output_string[3] = 0;
        output_string[2] = 0;
        output_string[1] = 0;
        output_string[0] = output_utf_32;
    }
    printf("%s", output_string);
}



// Runs the program
void run_program(unsigned long next_states[], unsigned char will_move_pointer[], unsigned char outputs[], unsigned char inputs[], unsigned long number_of_states) {
    // Initializes variables used for tracking the current state and moving along the cells array
    unsigned long state = 0;
    size_t last_bit = 0;
    size_t capacity = 8;
    size_t current_bit = 0;
    unsigned char toggle_bit = 0b00000001;

    // Initializes the cells array, the array of bits on which Axios operates
    char* cells = calloc(256);
    if (cells == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return;
    }

    // Initializes variables used for input and the input queue
    rear = (struct node*) malloc(sizeof(struct node));
    rear->input_utf_32 = 0xFFFFFFFF;
    front = rear;
    unsigned char current_input_bit = 0;
    unsigned long input_queue_size = 0;

    // Initializes variables used for output
    unsigned long output_utf_32 = 0;
    unsigned long toggle_output = 0x00000001;
    unsigned char will_not_print_extra_line = 255;

    // Loops through all states until the termination state is reached
    while (state < number_of_states) {
        if (inputs[state] == 0)
            // Changes the current bit
            // Happens for all states unless user input is requested
            cells[current_bit/8] ^= toggle_bit;
        else {
            // Operates input queue, changing the current bit according to user input
            // Asks for user input when there are not enough UTF-32 encodings stored
            if (will_not_print_extra_line) {
                printf("\n");
                will_not_print_extra_line = 0;
            }
            for (unsigned char i = inputs[state]; i > 0; i--) {
                while (input_queue_size * 21 < i) {
                    unsigned char input_string[1024];
                    fgets((char *) input_string, 1024, stdin);
                    input_queue_size = add_inputs(input_string, input_queue_size);
                    if (input_queue_size == 0xFFFFFFFF) {
                        clear_queue();
                        free(front);
                        return;
                    }
                }
                if (current_input_bit == 21) {
                    remove_front();
                    current_input_bit = 0;
                    input_queue_size--;
                }
                if ((front->input_utf_32 >> current_input_bit) & 1) {
                    if (!((cells[current_bit/8] >> current_bit%8) & 1))
                        cells[current_bit/8] ^= toggle_bit;
                } else {
                    if ((cells[current_bit/8] >> current_bit%8) & 1)
                        cells[current_bit/8] ^= toggle_bit;
                }
                current_input_bit++;
            }
        }

        // Outputs characters based on the number of 2 operators in the current state
        if (outputs[state] != 0) {
            for (unsigned char i = outputs[state]; i > 0; i--) {
                if ((cells[current_bit/8] >> current_bit%8) & 1)
                    output_utf_32 ^= toggle_output;
                if (toggle_output != 0x00100000)
                    toggle_output *= 2;
                else {
                    toggle_output = 0x00000001;
                    if (output_utf_32 != 0x1FFFFF) {
                        if (will_not_print_extra_line) {
                            printf("\n");
                            will_not_print_extra_line = 0;
                        }
                        display_output(output_utf_32);
                    } else {
                        clear_queue();
                        current_input_bit = 0;
                        input_queue_size = 0;
                    }
                    output_utf_32 = 0;
                }
            }
        }

        // If the current state has no 0 operators, shift the pointer forward
        // If the last bit is reached, add a new bit and return to the beginning
        // When this process is completed, go to the next state written in code
        if (will_move_pointer[state]) {
            if (current_bit != last_bit) {
                current_bit++;
                if (toggle_bit != 0b10000000)
                    toggle_bit *= 2;
                else
                    toggle_bit = 0b00000001;
            } else {
                current_bit = 0;
                toggle_bit = 0b00000001;
                last_bit++;

                // Reallocates cells array if it requires more elements beyond its current capacity
                if (last_bit == capacity) {
                    size_t byte_capacity = capacity / 4;
                    capacity *= 2;
                    cells = (char*) realloc(cells, byte_capacity);
                    if (cells == NULL) {
                        fprintf(stderr, "Failed to allocate memory\n");
                        clear_queue();
                        free(front);
                        return;
                    }
                    for (size_t i = byte_capacity / 2; i < byte_capacity; i++)
                        cells[i] = 0;
                }
            }
            state++;
        }
        // If there are 0 operators and the current bit is one, go to the state marked by next_states
        else if ((cells[current_bit/8] >> current_bit%8) & 1)
            state = next_states[state];
        // If there are 0 operators and the current bit is zero, go to the next state written in code
        else
            state++;
    }

    clear_queue();
    free(rear);
    free(cells);
    if (!will_not_print_extra_line)
        printf("\n");
}



// Parse Code

// Deterimines the operator associated with a three-byte character
unsigned char identify_three_byte_operator(char string[], size_t index) {
    switch (string[index]) {
        case '\xE0': {
            switch (string[index+1]) {
                // Devanagari numerals
                case '\xA5': {
                    switch (string[index+2]) {
                        case '\xA6': return '0';
                        case '\xA7': return '1';
                        case '\xA8': return '2';
                        case '\xA9': return '3';
                }} break;

                // Bengali numerals
                case '\xA7': {
                    switch(string[index+2]) {
                        case '\xA6': return '0';
                        case '\xA7': return '1';
                        case '\xA8': return '2';
                        case '\xA9': return '3';
                }} break;

                // Tamil numerals
                case '\xAF': {
                    switch(string[index+2]) {
                        case '\xA6': return '0';
                        case '\xA7': return '1';
                        case '\xA8': return '2';
                        case '\xA9': return '3';
                }} break;

                // Thai numerals
                case '\xB9': {
                    switch(string[index+2]) {
                        case '\x90': return '0';
                        case '\x91': return '1';
                        case '\x92': return '2';
                        case '\x93': return '3';
                }} break;

                // Lao numerals
                case '\xBB': {
                    switch(string[index+2]) {
                        case '\x90': return '0';
                        case '\x91': return '1';
                        case '\x92': return '2';
                        case '\x93': return '3';
                }} break;

                // Tibetan numerals
                case '\xBC': {
                    switch(string[index+2]) {
                        case '\xA0': return '0';
                        case '\xA1': return '1';
                        case '\xA2': return '2';
                        case '\xA3': return '3';
                }} break;
        }} break;

        case '\xE1': {
            switch (string[index+1]) {
                // Burmese numerals
                case '\x81': {
                    switch (string[index+2]) {
                        case '\x80': return '0';
                        case '\x81': return '1';
                        case '\x82': return '2';
                        case '\x83': return '3';
                }} break;

                // Khmer numerals
                case '\x9F': {
                    switch (string[index+2]) {
                        case '\xA0': return '0';
                        case '\xA1': return '1';
                        case '\xA2': return '2';
                        case '\xA3': return '3';
                }} break;
        }} break;

        // Fullwidth numerals
        case '\xEF': {
            if (string[index+1] == '\xBC') {
                switch (string[index+2]) {
                    case '\x90': return '0';
                    case '\x91': return '1';
                    case '\x92': return '2';
                    case '\x93': return '3';
        }}} break;
    }

    return 'N';
}

// Deterimines the operator associated with a two-byte character
unsigned char identify_two_byte_operator(char string[], size_t index) {
    switch (string[index]) {
        // Eastern Arabic numerals
        case '\xD9': {
            switch (string[index+1]) {
                case '\xA0': return '0';
                case '\xA1': return '1';
                case '\xA2': return '2';
                case '\xA3': return '3';
            }
        } break;

        // Persian numerals
        case '\xDB': {
            switch (string[index+1]) {
                case '\xB0': return '0';
                case '\xB1': return '1';
                case '\xB2': return '2';
                case '\xB3': return '3';
            }
        } break;
    }

    return 'N';
}

// Initializes code arrays to be used by the run_program() function and calls run_program()
void read_program(char code[]) {
    unsigned long number_of_states = 1;
    size_t code_index = 0;
    unsigned char c = code[0];

    // Calculates the number of states needed to initialize the array
    while (c != '\0') {
        if (c > 0xEF)
            code_index += 4;
        else if (c > 0xDF) {
            if (identify_three_byte_operator(code, code_index) == '1')
                number_of_states++;
            code_index += 3;
        } else if (c > 0xBF) {
            if (identify_two_byte_operator(code, code_index) == '1')
                number_of_states++;
            code_index += 2;
        } else {
            if (c == '1')
                number_of_states++;
            code_index++;
        }
        c = code[code_index];
    }

    // Displays error message if there are too many states, which would cause the
    // the code arrays to use a lot of memory (currently limited to 6.5 megabytes)
    if (number_of_states > 524288) {
        fprintf(stderr, "Code uses too many states, exceeding limits on memory\n");
        return;
    }

    // Initializes code arrays used for running the program
    unsigned long next_states[number_of_states];
    unsigned char will_move_pointer[number_of_states];
    unsigned char outputs[number_of_states];
    unsigned char inputs[number_of_states];
    outputs[0] = 0;
    inputs[0] = 0;

    unsigned long number_of_zeroes = 0;
    unsigned long state_index = 0;
    code_index = 0;
    c = code[0];

    // Stores all values into the code arrays based on the submitted code
    while (c != '\0') {
        if (c > 0xEF)
            code_index += 4;
        else if (c > 0xDF) {
            c = identify_three_byte_operator(code, code_index);
            code_index += 3;
        } else if (c > 0xBF) {
            c = identify_two_byte_operator(code, code_index);
            code_index += 2;
        } else {
            code_index++;
        }

        switch (c) {
            case '0': {
                number_of_zeroes++;
            } break;

            case '1': {
                if (number_of_zeroes > 0) {
                    will_move_pointer[state_index] = 0;
                    next_states[state_index] = state_index - number_of_zeroes + 1;
                    while (next_states[state_index] > 524288)
                        next_states[state_index] += number_of_states + 1;
                } else
                    will_move_pointer[state_index] = 255;
                number_of_zeroes = 0;
                state_index++;
                outputs[state_index] = 0;
                inputs[state_index] = 0;
            } break;

            case '2': {
                if (++outputs[state_index] == 255) {
                    fprintf(stderr, "Too many output operators in one state\nTry separating them into multiple states\n");
                    return;
                }
            } break;

            case '3': {
                if (++inputs[state_index] == 255) {
                    fprintf(stderr, "Too many input operators in one state\nTry separating them into multiple states\n");
                    return;
                }
            } break;
        }
        c = code[code_index];
    }

    // Repetition of code from case '1', which is needed for every state
    if (number_of_zeroes > 0) {
        will_move_pointer[state_index] = 0;
        next_states[state_index] = state_index - number_of_zeroes + 1;
        while (next_states[state_index] > 524288)
            next_states[state_index] += number_of_states + 1;
    } else
        will_move_pointer[state_index] = 255;

    run_program(next_states, will_move_pointer, outputs, inputs, number_of_states);
}



// Input Code

// Opens file, records the code contained inside, and calls read_program()
void run_code_from_file() {
    printf("\nType the name of your file: ");
    char file_name[256];
    fgets(file_name, 2, stdin); // Buffer function needed, or no user input will be read
    fgets(file_name, 256, stdin);

    // Replaces the newline character at the end of file_name with the null character, so the correct file name will be passed to fopen()
    for (int i = 0; i < 256; i++) {
        if (file_name[i] == '\n') {
            file_name[i] = '\0';
            break;
        }
    }
    file_name[255] = '\0'; // Ensures the last character is the null character in case the user's input exceeded the length of file_name

    FILE *file;
    file = fopen(file_name, "r");
    if (file == NULL) {
        fprintf(stderr, "File not found\n");
        return;
    }

    char code[131072] = {0};
    size_t code_index = 0;

    while ((code[code_index] = fgetc(file)) != EOF) {
        if (++code_index == 131072) {
            fprintf(stderr, "Code is too long, exceeding limits on memory");
            return;
        }
    }

    fclose(file);
    read_program(code);
}

// Runs the shell and calls read_program() until "2" is entered
void run_shell() {
    char code[131072] = {0};
    size_t code_index = 0;
    fgets(code, 2, stdin); // Buffer function needed, or no user input will be read
    printf("\n>>> ");
    fgets(code, 131072, stdin);

    while ((code[0] != '2' || code[1] != '\n')  && (identify_two_byte_operator(code, 0) != '2' || code[2] != '\n') && (identify_three_byte_operator(code, 0) != '2' || code[3] != '\n')) {
        if (code[131071] == '\0')
            read_program(code);
        else
            fprintf(stderr, "Code is too long, exceeding limits on memory");

        code_index = 0;
        while (code_index < 131072) {
            code[code_index] = '\0';
            code_index++;
        }
        printf("\n>>> ");
        fgets(code, 131072, stdin);
    }
}



// Runs menu prompt and calls run_shell() and run_file() functions
int main() {
    char selection[4] = {0};
    unsigned char numeral = 'N';
    do {
        printf("\nMake your selection by typing the associated number:\n");
        printf("[0] Run program from file\n");
        printf("[1] Run shell (type and enter \"2\" to exit)\n");
        printf("[2] License and further information\n");
        printf("[3] Quit\n");
        printf("Enter your choice: ");

        scanf("%4s", selection);
        numeral = selection[0];

        if (numeral > 0xDF && numeral < 0xF0)
            numeral = identify_three_byte_operator(selection, 0);
        else if (numeral > 0xBF)
            numeral = identify_two_byte_operator(selection, 0);

        switch (numeral) {
            case '0': {
                run_code_from_file();
            } break;

            case '1': {
                run_shell();
            } break;

            case '2': {
                printf("\nMIT License  Copyright (c) 2022  Maxine Dobbs\n");
                printf("For more information, check out this link: https://github.com/MaxDobbs32/Axios\n");
                printf("The full license text can be found at this link: https://github.com/MaxDobbs32/Axios/blob/main/LICENSE.md\n");
            } break;
        }
        selection[1] = 0; selection[2] = 0; selection[3] = 0;
    } while (numeral != '3');
    return 0;
}

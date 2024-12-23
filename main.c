#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum stopping_reason_t {
    HALTED,
    SURPASSED_MAX_STEPS,
    REACHED_LEFT_EDGE,
    REACHED_RIGHT_EDGE
} stopping_reason_t;

typedef struct tm_delta {
    unsigned char output;
    char dir; // L or R
    char nextstate;
} tm_delta;

typedef struct tmch {
    char state;
    size_t tape_len; // BYTES
    long long int head; // BITS
    tm_delta *table;
    unsigned char *tape;
} tmch;

tm_delta *tmch_step(tmch *tm)
{
    int state_index = tm->state - 'A';
    size_t byte_index = tm->head / 8;
    size_t bit_offset = tm->head % 8;
    unsigned char byte = tm->tape[byte_index];
    unsigned char input = (byte >> bit_offset) & 0x01;
    int table_index = 2 * state_index + input;
    tm_delta *delta = &tm->table[table_index];
    unsigned char output_mask = delta->output << bit_offset;
    unsigned char new_byte = byte | output_mask;
    tm->tape[byte_index] = new_byte;
    switch (delta->dir) {
    case 'L': tm->head--; break;
    case 'R': tm->head++; break;
    default:
        fprintf(stderr, "default in tmch_step\n");
        exit(EXIT_FAILURE);
    }
    tm->state = delta->nextstate;
    return delta;
}

void tm_print(tmch *tm) {
    for (int i = 0; i < tm->tape_len; i++) {
        if (i % 12 == 0) {
            printf("\n%04X    ", i);
        }
        if (tm->head / 8 == i) {
            printf("(%lld)", tm->head % 8);
        }
        else {
            printf("  ");
        }
        unsigned char reversed = 0;
        for (int j = 0; j < 8; j++) {
            unsigned char ith_bit = (tm->tape[i] >> j) & 0x01;
            reversed |= ith_bit << (7 - j);
        }
        printf("%02X ", reversed);
    }
    printf("\n");
}

int main (int argc, char **argv)
{
    unsigned long long int num_steps = 0ULL;
    const unsigned long long int max_steps = ~0 & ~(1ULL << 63);
    stopping_reason_t stopping_reason;

    // Transition table (aka delta function)
    tm_delta my_table[] = {
        {1, 'R', 'B'}, {1, 'L', 'C'},
        {1, 'R', 'C'}, {1, 'R', 'B'},
        {1, 'R', 'D'}, {0, 'L', 'E'},
        {1, 'L', 'A'}, {1, 'L', 'D'},
        {1, 'R', 'Z'}, {0, 'L', 'A'},
    };

    // Initialize turing machine
    tmch tm = {'A', 1 << 10, (1 << 9) * 8, my_table};
    tm.tape = calloc(tm.tape_len, sizeof(unsigned char));
    printf("Started TM with %ld bytes.\n", tm.tape_len);

    // Emulate turing machine
    char c = '\0'; // Terminal input
    printf("The bits are least-significant first.\n"
           "Press enter to step, r to run, or q to quit.\n");
    while (true) {
        // Stopping cases
        if (tm.state == 'Z') {
            stopping_reason = HALTED; break;
        }
        if (num_steps >= max_steps) {
            stopping_reason = SURPASSED_MAX_STEPS; break;
        }
        if (tm.head < 0) {
            stopping_reason = REACHED_LEFT_EDGE; break;
        }
        if (tm.head >= tm.tape_len * 8) {
            stopping_reason = REACHED_RIGHT_EDGE; break;
        }
        // Handle input and execute TM step
        if (c != 'r') {
            c = getchar();
            if (c == 'q') exit(EXIT_SUCCESS);
            tm_delta *my_delta = tmch_step(&tm);
            num_steps++;
            tm_print(&tm);
            printf("Wrote %d, moved %s, into state %c.\n",
                my_delta->output,
                my_delta->dir == 'L' ? "left" : "right",
                my_delta->nextstate
            );
            fflush(stdout);
        }
        else {
            tmch_step(&tm);
            num_steps++;
        }
    }

    // Print end tape contents
    if (c == 'r') {
        tm_print(&tm);
    }

    // Print halting reason
    switch (stopping_reason) {
    case HALTED:
        printf("Turing machine halted after %lld steps.\n", num_steps);
        break;
    case SURPASSED_MAX_STEPS:
        printf("Turing machine terminated after max steps %lld\n", max_steps);
        break;
    case REACHED_LEFT_EDGE:
        fprintf(stderr, "Head ran off the left after %lld steps.\n", num_steps);
        break;
    case REACHED_RIGHT_EDGE:
        fprintf(stderr, "Head ran off the right after %lld steps.\n", num_steps);
        break;
    }

    return 0;
}

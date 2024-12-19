#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

static unsigned long long int num_steps = 0;

typedef struct tm_delta {
    unsigned char output;
    char dir; // L or R
    char nextstate;
} tm_delta;

typedef struct tmch {
    char state;
    size_t strip_len; // BYTES
    long long int head; // BITS
    tm_delta *table;
    unsigned char *strip;
} tmch;

void tmch_step(tmch *tm)
{
    int state_index = tm->state - 'A';
    size_t byte_index = tm->head / 8;
    size_t bit_offset = tm->head % 8;
    unsigned char byte = tm->strip[byte_index];
    unsigned char input = (byte >> bit_offset) & 0x01;
    int delta_index = 2 * state_index + input;
    tm_delta *delta = &tm->table[delta_index];
    unsigned char output_mask = delta->output << bit_offset;
    unsigned char new_byte = byte | output_mask;
    tm->strip[byte_index] = new_byte;
    switch (delta->dir) {
    case 'L':
        if (tm->head <= 0) {
            fprintf(stderr, "Head ran off the left after %lld steps.\n", num_steps);
            exit(EXIT_FAILURE);
        }
        tm->head--;
        break;
    case 'R':
        if (tm->head >= tm->strip_len * 8 - 1) {
            fprintf(stderr, "Head ran off the right after %lld steps.\n", num_steps);
            exit(EXIT_FAILURE);
        }
        tm->head++;
        break;
    default:
        fprintf(stderr, "default in tmch_step\n");
        exit(EXIT_FAILURE);
    }
    tm->state = delta->nextstate;
}

int main(int argc, char **argv)
{
    // Transition table (aka delta function)
    tm_delta my_table[] = {
        {1, 'R', 'B'}, {1, 'L', 'C'},
        {1, 'R', 'C'}, {1, 'R', 'B'},
        {1, 'R', 'D'}, {0, 'L', 'E'},
        {1, 'L', 'A'}, {1, 'L', 'D'},
        {1, 'R', 'Z'}, {0, 'L', 'A'},
    };

    // Initialize turing machine
    tmch tm = {'A', 1 << 14, (1 << 13) * 8, my_table};
    tm.strip = calloc(tm.strip_len, sizeof(unsigned char));
    printf("Started turing machine with %ld bytes\n", tm.strip_len);

    // Emulate turing machine
    while (tm.state != 'Z') {
        tmch_step(&tm);
        num_steps++;
    }

    // Print end strip contents
    for (int i = 0; i < tm.strip_len; i++) {
        printf("%02X ", tm.strip[i]);
        if (i % 32 == 31) {
            printf("\n");
        }
    }

    printf("Turing machine terminated after %lld steps.\n", num_steps);

    return 0;
}

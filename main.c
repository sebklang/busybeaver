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

int tmch_step(tmch *tm)
{
    int state_idx = tm->state - 'A';
    size_t byte_idx = tm->head / 8;
    size_t bit_offset = tm->head % 8;
    unsigned char byte = tm->strip[byte_idx];
    unsigned char input = (byte >> bit_offset) & 0x01;
    int step_index = 2 * state_idx + input;
    tm_delta *step = &tm->table[step_index];
    unsigned char output_mask = step->output << bit_offset;
    unsigned char new_byte = byte | output_mask;
    tm->strip[byte_idx] = new_byte;
    switch (step->dir) {
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
        fprintf(stderr, "default in tmch_step");
        return -1;
    }
    tm->state = step->nextstate;
    return 0;
}

int main(int argc, char **argv)
{
    tm_delta my_table[] = {
        {1, 'R', 'B'}, {1, 'L', 'C'},
        {1, 'R', 'C'}, {1, 'R', 'B'},
        {1, 'R', 'D'}, {0, 'L', 'E'},
        {1, 'L', 'A'}, {1, 'L', 'D'},
        {1, 'R', 'Z'}, {0, 'L', 'A'},
    };

    tm_delta my_table_sus[] = {
        {1, 'L', 'B'}, {1, 'R', 'C'},
        {1, 'L', 'C'}, {1, 'L', 'B'},
        {1, 'L', 'D'}, {0, 'R', 'E'},
        {1, 'R', 'A'}, {1, 'R', 'D'},
        {1, 'L', 'Z'}, {0, 'R', 'A'},
    };

    tmch my_tm = {'A', 1 << 14, (1 << 13) * 8, my_table};
    my_tm.strip = calloc(my_tm.strip_len, sizeof(unsigned char));
    printf("SUS %ld\n", my_tm.strip_len);

    while (my_tm.state != 'Z') {
//        printf("%c", my_tm.state);
        tmch_step(&my_tm);
        num_steps++;
    }
    printf("%c\n", my_tm.state);

    for (int i = 0; i < my_tm.strip_len; i++) {
        printf("%02X ", my_tm.strip[i]);
        if (i % 32 == 31) {
            printf("\n");
        }
    }

    printf("Turing machine terminated after %lld steps.\n", num_steps);

    return 0;
}

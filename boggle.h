#ifndef BOGGLE_H
#define BOGGLE_H

#define BOGGLE_BOARD_WIDTH 4
#define BOGGLE_BOARD_HEIGHT 4
#define BOGGLE_ALPHABET_SIZE 26
#define BOGGLE_NUM_DICE (BOGGLE_BOARD_WIDTH * BOGGLE_BOARD_HEIGHT)
#define BOGGLE_MAX_WORD_LENGTH (BOGGLE_NUM_DICE + 1)
#define BOGGLE_NUM_DICE_SIDES 6
#define BOGGLE_NUM_LETTERS (BOGGLE_NUM_DICE_SIDES * BOGGLE_NUM_DICE)

typedef struct _boggle_t boggle_t;

boggle_t* boggle_new( void );
void boggle_init( boggle_t* bog );
void boggle_init_static( boggle_t* bog, const char* cs, int w, int h );
void boggle_set_verbose( boggle_t* bog, int verbose, int thresh );
void boggle_solve( boggle_t* bog, int* counts, int* scores );
void boggle_fini( boggle_t* bog );
void boggle_delete( boggle_t* bog );

void boggle_dbg_break( boggle_t* bog, int row, int col );

#endif

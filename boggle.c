#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "dict.h"

#define cassert(expr) extern char (*__c_assert(void)) [sizeof(char[1-2*!(expr)])]

#define BOGGLE_BOARD_WIDTH 4
#define BOGGLE_BOARD_HEIGHT 4
#define BOGGLE_ALPHABET_SIZE 26
#define BOGGLE_NUM_DICE (BOGGLE_BOARD_WIDTH * BOGGLE_BOARD_HEIGHT)
#define BOGGLE_MAX_WORD_LENGTH (BOGGLE_NUM_DICE + 1)
#define BOGGLE_NUM_DICE_SIDES 6
#define BOGGLE_NUM_LETTERS (BOGGLE_NUM_DICE_SIDES * BOGGLE_NUM_DICE)

typedef enum {
	BOGGLE_DIR_FIRST = 0,
	BOGGLE_DIR_N = 0,
	BOGGLE_DIR_NE = 1,
	BOGGLE_DIR_E = 2,
	BOGGLE_DIR_SE = 3,
	BOGGLE_DIR_S = 4,
	BOGGLE_DIR_SW = 5,
	BOGGLE_DIR_W = 6,
	BOGGLE_DIR_NW = 7,
	BOGGLE_DIR_LAST = BOGGLE_DIR_NW,
	BOGGLE_DIR_MAX
} dir_t;

typedef struct _boggle_t {
	char board[BOGGLE_BOARD_HEIGHT][BOGGLE_BOARD_WIDTH];
	dawg_t* dawg;

	int verbose;
	int verbose_thresh;

	int debug;
	int debug_row;
	int debug_col;
} boggle_t;

typedef struct _query_answers_t {
	int scores[BOGGLE_MAX_WORD_LENGTH];
	int counts[BOGGLE_MAX_WORD_LENGTH];
} query_answers_t;

typedef struct _query_stack_t {
	int size;
	char letters[BOGGLE_MAX_WORD_LENGTH + 1];
} query_stack_t;

typedef struct _query_t {
	boggle_t* bog;
	char fill[BOGGLE_BOARD_HEIGHT][BOGGLE_BOARD_WIDTH];
	query_stack_t stack;
        query_answers_t answers;
} query_t;

/* Standard Fisher-Yates/Knuth */
static void shuffle( char* cs, size_t n )
{
	int i;
	int j;
	char t;

	for( i = n - 1; i > 0; --i )
	{
		j = rand() % ( i + 1 );
		t = cs[j];
		cs[j] = cs[i];
		cs[i] = t;
	}
}

static void boggle_break( void )
{
	__asm__ volatile( "int $3\n" );
}

static int boggle_score( const char* word )
{
	/*
	 * Scoring:
	 * 3 = 1pt
	 * 4 = 1pt
	 * 5 = 2pt
	 * 6 = 3pt
	 * 7 = 5pt
	 * 8 = 11pt
	 */
	const int scoring[18] = {
		0,              /* 0 */
		0, 0, 1, 1,     /* 1, 2, 3, 4 */
		2, 3, 5, 11,    /* 5, 6, 7, 8 */
		11, 11, 11, 11, /* 9, 10, 11, 12 */
		11, 11, 11, 11, /* 13, 14, 15, 16 */
		11              /* 17 */
	};
	int n;
	int longest;
	int score;

	cassert( sizeof( scoring ) == sizeof( int ) * ( BOGGLE_NUM_DICE + 2 ) );

	n = strlen( word );
	longest = BOGGLE_MAX_WORD_LENGTH;
	n = n > longest ? longest : n;
	score = scoring[n];

	return score;
}

static void boggle_print_board( boggle_t* bog )
{
	int row;
	int col;

	printf( "board = {\n" );
	
	for( row = 0; row < BOGGLE_BOARD_HEIGHT; ++row )
	{
		for( col = 0; col < BOGGLE_BOARD_WIDTH; ++col )
		{
			printf( "%c ", bog->board[row][col] );
		}

		printf( "\n" );
	}

	printf( "}\n" );
}

boggle_t* boggle_new( void )
{
	boggle_t* bog;
	dict_t* dict;

	bog = (boggle_t*) malloc( sizeof( boggle_t ) );
	assert( bog );
	memset( bog, 0, sizeof( boggle_t ) );
	dict = dict_new();
	dict_init_file( dict, "yawl.dict", 600000 );
	bog->dawg = dawg_new();
	dict_finalize_to_dawg( dict, bog->dawg );
	dict_delete( dict );

	return bog;
}

void boggle_delete( boggle_t* bog )
{
	assert( bog );
	dawg_delete( bog->dawg );
	free( bog );
}


/*
 * From http://www.boardgamegeek.com/thread/300565/
 *
 * Distribution:
 * A: vi | viii | -2
 * B: ii | iii | -1 
 * C: ii | iii | -1 
 * D: iii | iiii | -1 
 * E: xi | x | +1 
 * F: ii | ii | . 
 * G: ii | iii | -1 
 * H: v | iii | +2 
 * I: vi | vii | -1 
 * J: i | i | . 
 * K: i | ii | -1 
 * L: iiii | v | -1 
 * M: ii | iii | -1 
 * N: vi | v | +1 
 * O: vii | vi | +1 
 * P: ii | iii | -1 
 * Q: i | i | . 
 * R: v | iiii | +1 
 * S: vi | v | +1 
 * T: viiii | v | +4 
 * U: iii | iiii | -1 
 * V: ii | ii | . 
 * W: iii | ii | +1 
 * X: i | i | . 
 * Y: iii | iii | . 
 * Z: i | i | . 
 */
void boggle_init( boggle_t* bog )
{
	const int frequency[26] = {
		6, 2, 2, 3,  /* A B C D */
		11, 2, 2, 5, /* E F G H */
		6, 1, 1, 4,  /* I J K L */
		2, 6, 7, 2,  /* M N O P */
		1, 5, 6, 9,  /* Q R S T */
		3, 2, 3, 1,  /* U V W X */
		3, 1         /* Y Z */
	};
	char alphabet[BOGGLE_NUM_LETTERS + 1];
	int i;
	int j;
	int n;
	int row;
	int col;

	cassert( BOGGLE_ALPHABET_SIZE == 26 );
	cassert( sizeof( frequency ) == sizeof( int ) * BOGGLE_ALPHABET_SIZE );
	assert( frequency['Q' - 'A'] == 1 );

	for( i = 0, n = 0; i < BOGGLE_ALPHABET_SIZE; i++ )
	{
		for( j = 0; j < frequency[i]; j++ )
		{
			assert( n < BOGGLE_NUM_LETTERS );
			alphabet[n++] = 'A' + i;
		}
	}

	assert( n == BOGGLE_NUM_LETTERS );
	alphabet[ BOGGLE_NUM_LETTERS - 1 ] = 0;
	shuffle( alphabet, BOGGLE_NUM_LETTERS - 1 );

	for( row = 0, n = 0; row < BOGGLE_BOARD_HEIGHT; ++row )
	{
		for( col = 0; col < BOGGLE_BOARD_WIDTH; ++col )
		{
			bog->board[row][col] = alphabet[n++];
		}
	}

	boggle_print_board( bog );
}

void boggle_init_static( boggle_t* bog, const char* cs, int w, int h )
{
	assert( w == BOGGLE_BOARD_WIDTH );
	assert( h == BOGGLE_BOARD_HEIGHT );

	memcpy( bog->board, cs, BOGGLE_BOARD_WIDTH * BOGGLE_BOARD_HEIGHT );
	boggle_print_board( bog );
}

void boggle_set_verbose( boggle_t* bog, int verbose, int thresh )
{
	bog->verbose = verbose;
	bog->verbose_thresh = thresh;
}

void boggle_dbg_break( boggle_t* bog, int row, int col )
{
	bog->debug = 1;
	bog->debug_row = row;
	bog->debug_col = col;
}

void boggle_fini( boggle_t* bog )
{
	/* empty */
}

static void query_init( query_t* query, boggle_t* bog )
{
	memset( query, 0, sizeof( query_t ) );
	query->bog = bog;
}

static void query_fini( query_t* query )
{
	/* empty */
}

static int query_dir_valid( query_t* query, int row, int col, dir_t dir, int* rowo, int* colo )
{
	const int deltas[BOGGLE_DIR_MAX][2] = {
		{ -1, 0 },  /* BOGGLE_DIR_N */
		{ -1, 1 },  /* BOGGLE_DIR_NE */
		{ 0, 1 },   /* BOGGLE_DIR_E  */
		{ 1, 1 },   /* BOGGLE_DIR_SE */
		{ 1, 0 },   /* BOGGLE_DIR_S */
		{ 1, -1 },  /* BOGGLE_DIR_SW */
		{ 0, -1 },  /* BOGGLE_DIR_W */
		{ -1, -1 }, /* BOGGLE_DIR_NW */
	};
	int rowp;
	int colp;
	int rowv;
	int colv;
	int valid;
	int occupied;

	rowp = row + deltas[dir][0];
	colp = col + deltas[dir][1];
	rowv = ( rowp >= 0 ) && ( rowp < BOGGLE_BOARD_HEIGHT );
	colv = ( colp >= 0 ) && ( colp < BOGGLE_BOARD_WIDTH );
	valid = rowv && colv;

	if( valid )
	{
		occupied = query->fill[rowp][colp] != 0;

		if( !occupied )
		{
			*rowo = rowp;
			*colo = colp;
			return 1;
		}
	}

	return 0;
}

static void boggle_solve_r( query_t* query, int row, int col )
{
	dir_t dir;
	char at;
	int rowp;
	int colp;
	int found;
	int marked;
	int score;
	int valid;

	assert( row < BOGGLE_BOARD_HEIGHT );
	assert( col < BOGGLE_BOARD_WIDTH );
	at = query->bog->board[row][col];
	assert( query->stack.size <= BOGGLE_MAX_WORD_LENGTH );
	query->stack.letters[query->stack.size] = at;
	query->stack.size += 1;
	query->fill[row][col] = at;

	if( query->stack.size >= 3 )
	{
		found = dawg_find( query->bog->dawg, query->stack.letters );

		if( found >= 0 )
		{
			marked = dawg_mark( query->bog->dawg, found );

			if( marked == 0 )
			{
				score = boggle_score( query->stack.letters );
				if( query->bog->verbose && query->stack.size >= query->bog->verbose_thresh )
				{
					printf( "found valid word '%s' (%d)\n", query->stack.letters, score );
				}
				query->answers.counts[query->stack.size] += 1;
				query->answers.scores[query->stack.size] += score;
			}
			else
			{
				if( query->bog->verbose )
				{
					printf( "found valid word '%s' but skipping as already counted\n", query->stack.letters );
				}
			}
		}
	}

	for( dir = BOGGLE_DIR_FIRST; dir <= BOGGLE_DIR_LAST; ++dir )
	{
		valid = query_dir_valid( query, row, col, dir, &rowp, &colp );

		if( valid )
		{
			boggle_solve_r( query, rowp, colp );
		}
	}

	assert( row < BOGGLE_BOARD_HEIGHT );
	assert( col < BOGGLE_BOARD_WIDTH );
	query->fill[row][col] = 0;
	query->stack.size -= 1;
	assert( query->stack.size >= 0 );
	query->stack.letters[query->stack.size] = 0;
}

void boggle_solve( boggle_t* bog )
{
	query_t query;
	int row;
	int col;
	int i;
	int count;
	int score;
	int total_count;
	int total_score;

	query_init( &query, bog );

	for( row = 0; row < BOGGLE_BOARD_HEIGHT; ++row )
	{
		for( col = 0; col < BOGGLE_BOARD_WIDTH; ++col )
		{
			if( bog->debug )
			{
				if( bog->debug_row == row && bog->debug_col == col )
				{
					boggle_break();
				}
			}

			boggle_solve_r( &query, row, col );
		}
	}

	total_count = 0;
	total_score = 0;

	for( i = 0; i < BOGGLE_MAX_WORD_LENGTH; ++i )
	{
		if ( !query.answers.counts[i] )
		{
			continue;
		}
		count = query.answers.counts[i];
		score = query.answers.scores[i];
		printf( "%8d %2d-letter words (%4d points)\n", count, i, score );
		total_count += count;
		total_score += score;
	}

	printf( "%8d total points, %4d total words.\n", total_score, total_count );

	query_fini( &query );
}

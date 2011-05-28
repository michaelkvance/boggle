#ifndef BOGGLE_H
#define BOGGLE_H

typedef struct _boggle_t boggle_t;

boggle_t* boggle_new( void );
void boggle_init( boggle_t* bog );
void boggle_init_static( boggle_t* bog, const char* cs, int w, int h );
void boggle_set_verbose( boggle_t* bog, int verbose, int thresh );
void boggle_solve( const boggle_t* bog );
void boggle_fini( boggle_t* bog );
void boggle_delete( boggle_t* bog );

void boggle_dbg_break( boggle_t* bog, int row, int col );

#endif

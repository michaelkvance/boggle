#ifndef DICT_H
#define DICT_H

typedef struct _dict_t dict_t;

dict_t* dict_new();
void dict_delete( dict_t* dict );
void dict_init_file( dict_t* dict, const char* filename, int hint );
int dict_find( dict_t* dict, const char* needle );
int dict_mark( dict_t* dict, int idx );
void dict_analyze( dict_t* dict );
void dict_finalize( dict_t* dict );

typedef struct _dawg_t dawg_t;

dawg_t* dawg_new();
void dawg_delete( dawg_t* dawg );
int dawg_find( dawg_t* dawg, const char* needle );
int dawg_mark( dawg_t* dawg, int idx );
int dawg_root( dawg_t* dawg );
int dawg_child( dawg_t* dawg, int idx, char c );

void dict_finalize_to_dawg( dict_t* dict, dawg_t* dawg );

#endif

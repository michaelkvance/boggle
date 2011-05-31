#include <assert.h>

#include "dict.h"

int main( int argc, char* argv[] )
{
	dict_t* dict;
	int r;
	dawg_t* dawg;

	dict = dict_new();
	dict_init_file( dict, "tiny.dict", 13 );
	r = dict_find( dict, "aaa" );
	assert( r >= 0 );
	r = dict_find( dict, "aab" );
	assert( r >= 0 );
	r = dict_find( dict, "aaaa" );
	assert( r >= 0 );
	r = dict_find( dict, "bbb" );
	assert( r >= 0 );
	r = dict_find( dict, "ababa" );
	assert( r >= 0 );
	r = dict_find( dict, "a" );
	assert( r < 0 );
	r = dict_find( dict, "aa" );
	assert( r < 0 );
	r = dict_find( dict, "aabb" );
	assert( r < 0 );
	r = dict_find( dict, "xxx" );
	assert( r < 0 );
	dict_delete( dict );

	dict = dict_new();
	dict_init_file( dict, "yawl.dict", 1000000 );
	dict_analyze( dict );
	dict_finalize( dict );
	r = dict_find( dict, "overelaborate" );
	assert( r >= 0 );
	r = dict_find( dict, "zdasfghk" );
	assert( r < 0 );
	r = dict_find( dict, "zzz" );
	assert( r >= 0 );
	r = dict_find( dict, "motherfuckermotherfucker" );
	assert( r < 0 );
	dawg = dawg_new();
	dict_finalize_to_dawg( dict, dawg );
	dict_delete( dict );
	r = dawg_find( dawg, "overelaborate" );
	assert( r >= 0 );
	r = dawg_find( dawg, "zdasfghk" );
	assert( r < 0 );
	r = dawg_find( dawg, "zzz" );
	assert( r >= 0 );
	r = dawg_find( dawg, "motherfuckermotherfucker" );
	assert( r < 0 );
	dawg_delete( dawg );

	return 0;
}

#include <assert.h>

#include "dict.h"

int main( int argc, char* argv[] )
{
	dict_t* dict;
	int r;
	dawg_t* dawg;

	dict = dict_new();
	dict_init_file( dict, "tiny.dict", 100 );
	dict_analyze( dict );
	dict_finalize( dict );
	r = dict_find( dict, "AAA" );
	assert( r >= 0 );
	r = dict_find( dict, "AAB" );
	assert( r >= 0 );
	r = dict_find( dict, "AAAA" );
	assert( r >= 0 );
	r = dict_find( dict, "BBB" );
	assert( r >= 0 );
	r = dict_find( dict, "ABABA" );
	assert( r >= 0 );
	r = dict_find( dict, "A" );
	assert( r < 0 );
	r = dict_find( dict, "AA" );
	assert( r < 0 );
	r = dict_find( dict, "AABB" );
	assert( r < 0 );
	r = dict_find( dict, "XXX" );
	assert( r < 0 );
	dawg = dawg_new();
	dict_finalize_to_dawg( dict, dawg );
	dict_delete( dict );
	dawg_analyze( dawg );
	r = dawg_find( dawg, "AAA" );
	assert( r >= 0 );
	r = dawg_find( dawg, "AAB" );
	assert( r >= 0 );
	r = dawg_find( dawg, "AAAA" );
	assert( r >= 0 );
	r = dawg_find( dawg, "BBB" );
	assert( r >= 0 );
	r = dawg_find( dawg, "ABABA" );
	assert( r >= 0 );
	r = dawg_find( dawg, "A" );
	assert( r < 0 );
	r = dawg_find( dawg, "AA" );
	assert( r < 0 );
	r = dawg_find( dawg, "AABB" );
	assert( r < 0 );
	r = dawg_find( dawg, "XXX" );
	assert( r < 0 );
	dawg_delete( dawg );

	dict = dict_new();
	dict_init_file( dict, "yawl.dict", 1000000 );
	dict_analyze( dict );
	dict_finalize( dict );
	r = dict_find( dict, "OVERELABORATE" );
	assert( r >= 0 );
	r = dict_find( dict, "ZDASFGHK" );
	assert( r < 0 );
	r = dict_find( dict, "ZZZ" );
	assert( r >= 0 );
	r = dict_find( dict, "MOTHERFUCKERMOTHERFUCKER" );
	assert( r < 0 );
	dawg = dawg_new();
	dict_finalize_to_dawg( dict, dawg );
	dict_delete( dict );
	dawg_analyze( dawg );
	r = dawg_find( dawg, "OVERELABORATE" );
	assert( r >= 0 );
	r = dawg_find( dawg, "ZDASFGHK" );
	assert( r < 0 );
	r = dawg_find( dawg, "ZZZ" );
	assert( r >= 0 );
	r = dawg_find( dawg, "MOTHERFUCKERMOTHERFUCKER" );
	assert( r < 0 );
	dawg_delete( dawg );

	return 0;
}

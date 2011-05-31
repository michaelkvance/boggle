#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DICT_NODE_NUM_CHILDREN 26

#define DICT_NODE_FLAG_WORD 0x1
#define DICT_NODE_FLAG_MARK 0x2

#define DICT_CHILD_IDX(c) ((c) - 'A')

#define DAWG_NODE_FLAG_WORD DICT_NODE_FLAG_WORD
#define DAWG_NODE_FLAG_MARK DICT_NODE_FLAG_MARK

#define DAWG_CHILD_IDX(c) ((c) - 'A')

typedef struct _dict_node_t {
	int children[DICT_NODE_NUM_CHILDREN];
	unsigned int set;
	unsigned int flags;
} dict_node_t;

typedef struct _dict_t {
	dict_node_t* nodes;
	unsigned int size;
	unsigned int avail;
	unsigned int root;
} dict_t;

typedef struct _dawg_child_t {
	unsigned int letter : 5;
	unsigned int index : 27;
} dawg_child_t;

typedef struct _dawg_node_t {
	dawg_child_t* children;
	unsigned int count : 5;
	unsigned int flags : 27;
} dawg_node_t;

typedef struct _dawg_t {
	dawg_node_t* nodes;
	unsigned int size;
	unsigned int root;
	dawg_child_t* store;
	unsigned int alloced;
} dawg_t;

dict_t* dict_new( void )
{
	dict_t *dict;

	dict = (dict_t*)malloc( sizeof( dict_t ) );
	memset( dict, 0, sizeof( dict_t ) );

	return dict;
}

void dict_delete( dict_t* dict )
{
	free( dict->nodes );
	free( dict );
}

void dict_node_init( dict_t* dict, dict_node_t* node )
{
	int i;

	for( i = 0; i < DICT_NODE_NUM_CHILDREN; i++ )
	{
		node->children[i] = -1;
	}

	node->set = 0;
	node->flags = 0;
}

static void dict_add_r( dict_t* dict, dict_node_t* node, const char* word )
{
	char c;
	int idx;
	dict_node_t* child;

	c = *word;

	if( c == '\0' )
	{
		node->flags |= DICT_NODE_FLAG_WORD;
		return;
	}

	if( node->children[DICT_CHILD_IDX(c)] < 0 )
	{
		assert( dict->avail < dict->size );
		idx = dict->avail;
		child = &dict->nodes[idx];
		dict->avail += 1;
		dict_node_init( dict, child );
		node->children[DICT_CHILD_IDX(c)] = idx;
		node->set += 1;
	}

	idx = node->children[DICT_CHILD_IDX(c)];
	assert( idx >= 0 );
	assert( idx < dict->size );
	dict_add_r( dict, &dict->nodes[idx], word + 1 );
}

void dict_add( dict_t* dict, const char* word )
{
	dict_node_t* node;

	node = &dict->nodes[dict->root];

	dict_add_r( dict, node, word );
}

static int dict_find_r( dict_t* dict, dict_node_t* node, const char* word )
{
	char c;
	int idx;
	dict_node_t* child;

	c = *word;

	if( c == '\0' )
	{
		return ( node->flags & DICT_NODE_FLAG_WORD ) ? node - dict->nodes : -1;
	}

	idx = node->children[DICT_CHILD_IDX(c)];

	if( idx < 0 )
	{
		return -1;
	}
	else
	{
		child = &dict->nodes[idx];
		return dict_find_r( dict, child, word + 1 );
	}
}

int dict_find( dict_t* dict, const char* word, int mark )
{
	dict_node_t* node;

	node = &dict->nodes[dict->root];

	return dict_find_r( dict, node, word );
}

int dict_mark( dict_t* dict, int idx )
{
	dict_node_t* node;
	int old;

	node = &dict->nodes[idx];
	old = node->flags;
	node->flags |= DICT_NODE_FLAG_MARK;

	return ( old & DICT_NODE_FLAG_MARK );
}

void dict_init_file( dict_t* dict, const char* filename, int hint )
{
	FILE* fp;
	int nwords;
	int c;
	char token[64];
	char* parsed;
	int l;

	fp = fopen( filename, "r" );

	if( !fp )
	{
		fprintf( stderr, "Boo!\n" );
		exit( -1 );
	}

	nwords = 0;

	while( !feof( fp ) )
	{
		c = fgetc( fp );
		nwords += ( c == '\n' ) ? 1 : 0;
	}

	assert( dict->avail == 0 );
	dict->size = hint;
	dict->nodes = (dict_node_t*)malloc( dict->size * sizeof( dict_node_t ) );
	dict->root = 0;
	dict_node_init( dict, &dict->nodes[dict->root] );
	dict->avail += 1;

	fseek( fp, SEEK_SET, 0 );

	while( !feof( fp ) )
	{
		parsed = fgets( token, sizeof( token ), fp );

		if( !parsed )
		{
			assert( !ferror( fp ) );
			break;
		}

		l = strlen( parsed );

		if( parsed[ l - 1 ] == '\n' )
		{
			parsed[ l - 1 ] = 0;
			l -= 1;
		}

		if( l < 3 )
		{
			continue;
		}

		dict_add( dict, parsed );
	}

	fclose( fp );
}

void dict_analyze( dict_t* dict )
{
	int empty;
	int i;
	int nonempty;

	printf( "dict->size = %d (%ld) dict->avail = %d (%ld)\n", dict->size, dict->size * sizeof( dict_node_t ), dict->avail, dict->avail * sizeof( dict_node_t ) );

	nonempty = 0;

	for( i = 0; i < dict->avail; i++ )
	{
		nonempty += dict->nodes[i].set;
	}

	empty = dict->avail * 26 - nonempty;
	printf( "dict has %d empty and %d non-empty child indices (%ld wasted bytes)\n", empty, nonempty, empty * sizeof(int) );
}

void dict_finalize( dict_t* dict )
{
	dict_node_t* final;

	if ( dict->avail == dict->size )
	{
		return;
	}

	final = (dict_node_t*)realloc( dict->nodes, dict->avail * sizeof( dict_node_t ) );
	if ( final )
	{
		dict->nodes = final;
	}
}

dawg_t* dawg_new( void )
{
	dawg_t* dawg;

	dawg = (dawg_t*)malloc( sizeof( dawg_t ) );
	memset( dawg, 0, sizeof( dawg_t ) );

	return dawg;
}

void dawg_delete( dawg_t *dawg )
{
	free( dawg->store );
	free( dawg->nodes );
	free( dawg );
}

void dict_finalize_to_dawg( dict_t* dict, dawg_t* dawg )
{
	int i;
	int nonempty;
	dict_node_t* inode;
	dawg_node_t* anode;
	int j;
	dawg_child_t* children;

	for( i = 0; i < dict->avail; i++ )
	{
		inode = &dict->nodes[i];
		nonempty += inode->set;
	}

	dawg->size = dict->avail;
	dawg->nodes = (dawg_node_t*)malloc( sizeof( dawg_node_t ) * dawg->size );
	dawg->root = 0;
	dawg->store = (dawg_child_t*)malloc( sizeof( dawg_child_t ) * nonempty );
	dawg->alloced = 0;

	for( i = 0; i < dict->avail; i++ )
	{
		inode = &dict->nodes[i];
		anode = &dawg->nodes[i];

		children = &dawg->store[dawg->alloced];
		dawg->alloced += inode->set;
		anode->children = children;
		anode->count = 0;
		// XXX cassert that flags values are the same
		anode->flags = inode->flags;

		for( j = 0; j < DICT_NODE_NUM_CHILDREN; j++ )
		{
			if( inode->children[j] >= 0 )
			{
				assert( anode->count < nonempty );
				anode->children[anode->count].letter = j;
				anode->children[anode->count].index = inode->children[j];
				anode->count += 1;
			}
		}
	}
}

static int dawg_find_r( dawg_t* dawg, dawg_node_t* node, const char* word )
{
	char c;
	int idx;
	int i;
	dawg_node_t* child;

	c = *word;

	if( c == '\0' )
	{
		return ( node->flags & DAWG_NODE_FLAG_WORD ) ? node - dawg->nodes : -1;
	}

	idx = -1;

	for( i = 0; i < node->count; i++ )
	{
		if ( node->children[i].letter == DAWG_CHILD_IDX(c) )
		{
			idx = node->children[i].index;
			break;
		}
	}

	if( idx < 0 )
	{
		return -1;
	}
	else
	{
		child = &dawg->nodes[idx];
		return dawg_find_r( dawg, child, word + 1 );
	}
}

int dawg_find( dawg_t* dawg, const char* word, int mark )
{
	dawg_node_t* node;

	node = &dawg->nodes[dawg->root];

	return dawg_find_r( dawg, node, word );
}

int dawg_mark( dawg_t* dawg, int idx )
{
	dawg_node_t* node;
	int old;

	node = &dawg->nodes[idx];
	old = node->flags;
	node->flags |= DAWG_NODE_FLAG_MARK;

	return ( old & DAWG_NODE_FLAG_MARK );
}

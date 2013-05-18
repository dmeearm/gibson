/*
 * Copyright (c) 2013, Simone Margaritelli <evilsocket at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Gibson nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "atree.h"
/*
 * Find next link with 'ascii' byte.
 */
atree_item_t *at_find_next_link( atree_t *at, char ascii ){
	int i, j, n_links = at->n_links, r_start = n_links - 1;

	for( i = 0, j = r_start; i < n_links; ++i, --j ){
		if( at->links[i]->ascii == ascii ){
			return at->links[i];
		}
		else if( at->links[j]->ascii == ascii ){
			return at->links[j];
		}
	}
	return NULL;
}

void at_append_link( atree_t *at, atree_item_t *link ){
	at->links = (atree_t **)realloc( at->links, sizeof(atree_t **) * at->n_links + 1 );
	at->links[ at->n_links++ ] = link;
}

void *at_insert( atree_t *at, char *key, int len, void *value ){
	/*
	 * End of the chain, set the marker value and exit the recursion.
	 */
	if(!len){
		void *old = at->e_marker;
		at->e_marker = value;
		return old;
	}
	/*
	 * Has the item a link with given byte?
	 */
	atree_item_t *link = at_find_next_link( at, key[0] );
	if( link ){
		/*
		 * Next recursion, search next byte,
		 */
		return at_insert( link, ++key, --len, value );
	}
	/*
	 * Nothing found.
	 */
	else{
		/*
		 * Allocate, initialize and append a new link.
		 */
		at_init_link( link, key );
		at_append_link( at, link );
		/*
		 * Continue with next byte.
		 */
		return at_insert( link, ++key, --len, value );
	}

	return NULL;
}

atree_t *at_find_node( atree_t *at, char *key, int len ){
	atree_item_t *link = at;
	int i = 0;

	do{
		/*
		 * Find next link ad continue.
		 */
		link = at_find_next_link( link, key[i++] );
	}
	while( --len && link );

	return link;
}

void *at_find( atree_t *at, char *key, int len ){
	atree_item_t *link = at_find_node( at, key, len );
	/*
	 * End of the chain, if e_marker is NULL this chain is not complete,
	 * therefore 'key' does not map any alive object.
	 */
	return ( link ? link->e_marker : NULL );
}

void at_recurse( atree_t *at, at_recurse_handler handler, void *data ) {
	size_t i;

	for( i = 0; i < at->n_links; i++ ){
		at_recurse( at->links[i], handler, data );
	}

	handler( at, data );
}

void at_search_recursive_handler(atree_item_t *node, void *data){
	llist_t *list = data;

	if( node->e_marker != NULL )
		ll_append( list, node );
}

llist_t* at_search( atree_t *at, char *prefix ) {
	llist_t *list = NULL;
	atree_item_t *start = at_find_node( at, prefix, strlen(prefix) );

	if( start ){
		list = ll_prealloc(255);

		at_recurse( start, at_search_recursive_handler, list );
	}

	return list;
}

void *at_remove( atree_t *at, char *key, int len ){
	atree_item_t *link = at;
	int i = 0;

	do{
		/*
		 * Find next link ad continue.
		 */
		link = at_find_next_link( link, key[i++] );
	}
	while( --len && link );
	/*
	 * End of the chain, if e_marker is NULL this chain is not complete,
	 * therefore 'key' does not map any alive object.
	 */
	if( link && link->e_marker ){
		void *retn = link->e_marker;

		link->e_marker = NULL;

		return retn;
	}
	else
		return NULL;
}

void at_free( atree_t *at ){
	int i, n_links = at->n_links;
	/*
	 * Better be safe than sorry ;)
	 */
	if( at->links ){
		/*
		 * First of all, loop all the sub links.
		 */
		for( i = 0; i < n_links; ++i, --at->n_links ){
			/*
			 * Free this link sub-links.
			 */
			at_free( at->links[i] );
			/*
			 * Free the link itself.
			 */
			free( at->links[i] );
		}
		/*
		 * Finally free the array.
		 */
		free( at->links );
		at->links = NULL;
	}
}

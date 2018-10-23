///@file
///@brief	Incremental module search test program
///@copyright	Copyright (C) 2018  Andre Colomb
///
/// This file is part of libdebugmod.  It serves as demo code and
/// implements an efficient incremental string search algorithm to
/// search for a module by name.  Incoming characters are matched
/// against the list of registered debug modules, always returning the
/// first (partial) match from the list.
///
/// The functions build_search_table(), reset_search() and
/// incremental_search() can be adapted and reused for appropriate
/// applications.  They shall serve as another, more elaborate example
/// what is possible with the debug_mod_list() API.  Debug output from
/// this test itself is disabled (DEBUG_MOD_ENABLE undefined) by
/// default, but the search progress is output to stdout.
///
/// libdebugmod is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Lesser General Public License as
/// published by the Free Software Foundation, either version 3 of the
/// License, or (at your option) any later version.
///
/// libdebugmod is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
/// Lesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public
/// License along with this program.  If not, see
/// <http://www.gnu.org/licenses/>.
///
///@author	Andre Colomb <src@andre.colomb.de>


// Uncomment this to always prevent debug builds of this module
#undef DEBUG_MOD_ENABLE
#include <debug_mod_control.h>

#include <string.h>



// Lazy initialization using source file name as identifier
DEBUG_MOD_INIT(__FILE__)



/// Short-hand to evaluate the number of array entries
#define ENTRIES(arr)	(sizeof(arr) / sizeof(*(arr)))

/// Convenience macro to output debug message
#define MSG(fmt, ...)		DEBUGF(fprintf, (fmt), ##__VA_ARGS__)

/// Input character that will restart the search
static const char search_terminator = '\n';



/// Cached mapping of a module ID with its string length
struct search_entry {
    /// Pointer to the original module identifier
    const char*		id;
    /// Identifier string length
    unsigned		len;
};
/// Cache table with known module IDs, same size as the module list
static struct search_entry search_table[DEBUG_MOD_MAX];

/// List / table index of the last matched module
static debug_mod_index_t mod_index;
/// Pointer just beyond the last matched character from the list or
/// NULL if no more matches
static const char* cursor;

/// Permanent reference to the registered modules list
static debug_mod* const *mods;



/// Scan the available module ID strings and index their length
static inline void
build_search_table(void)
{
    debug_mod_index_t i;

    // Invalidate the incremental search state before rebuilding cache
    mod_index = 0;
    cursor = NULL;
    // Copy identifier addresses and pre-determine string lengths
    for (i = 0; i < ENTRIES(search_table); ++i) {
	if (mods[i]) {		//valid list entry
	    search_table[i].id = mods[i]->module;
	    search_table[i].len = strlen(mods[i]->module);
	    MSG("%u=%s\n", i, mods[i]->module);
	    if (! cursor) cursor = search_table[i].id;
	} else {		//invalid entry
	    search_table[i].id = NULL;
	    search_table[i].len = 0;
	    MSG("%u empty\n", i);
	}
    }
}



/// Reset the incremental string search state
///
/// After this function is called, the search state points at the
/// first valid cache table entry, which by definition matches an
/// empty search criterion.  If the cache table has no valid entries,
/// the cursor consequently indicates "no match".
static inline void
reset_search(void)
{
    // Invalidate the incremental search state
    mod_index = 0;
    cursor = NULL;
    // Find the first valid cache table entry
    do {
	if (search_table[mod_index].id) {
	    cursor = search_table[mod_index].id;
	    break;
	}
    } while (++mod_index < ENTRIES(search_table));
    MSG("start index %u, cursor@%p\n", mod_index, cursor);
}



/// Update incremental search state for new incoming character
static inline void
incremental_search(
    const char c)	///< Next character to be appended to the search criterion
{
    MSG("new character %c, cursor@%p\n", c, cursor);

    // Careful: cursor can only ever point at or after the current search table entry's id
    unsigned progress = cursor - search_table[mod_index].id;

    if (progress < search_table[mod_index].len) {
	if (c == *cursor) {
	    MSG("\t%u: matched [+%u], advance\n", mod_index, progress);
	    // Advance pointer for next iteration
	    ++cursor;
	    return;
	} else {
	    MSG("\tskip: char mismatch (%c)\n", *cursor);
	}
    } else {
	MSG("\tskip: too short\n");
    }

    // Save currently matching string for later comparison
    const char *old_id = search_table[mod_index].id;
    // Advance to next module
    while (++mod_index < ENTRIES(search_table)) {
	MSG("\t%u: next mod @%p: %s\n",
	    mod_index, search_table[mod_index].id, search_table[mod_index].id);
	if (! search_table[mod_index].id) {
	    MSG("\tskip: mod empty\n");
	    continue;
	}
	if (search_table[mod_index].len < progress) {
	    MSG("\tskip: mod too short\n");
	    continue;
	}
	cursor = search_table[mod_index].id + progress;
	MSG("\t%u: new pos: [+%u] cursor@%p\n", mod_index, progress, cursor);
	if (c != *cursor) {
	    MSG("\tskip: char mismatch\n");
	    continue;
	}
	MSG("\tchar match, strncmp(%u)\n", progress);
	if (strncmp(old_id, search_table[mod_index].id, progress)) {
	    MSG("\tskip: prefix mismatch\n");
	    continue;
	}
	// Advance pointer for next iteration
	++cursor;
	return;
    }

    MSG("\t%u: end of modules, no match\n", mod_index);
    cursor = NULL;
}



///@brief Prefix debug output with function context
///@see debug_mod_f
char
context(debug_mod* restrict self,
	const char* restrict context)
{
    fputs(context, self->stream);
    fputs("()\t", self->stream);
    return 1;
}



void
search_init(void)
{
    debug_mod_index_t dummy;

    mods = debug_mod_list(&dummy);
    // Makes sure this module itself is registered and included in the search table
    debug_mod_register_self();
    // This would also implicitly trigger the registration
    MSG("list of %u mods\n", dummy);

    build_search_table();
}



/// Test program for incremental search algorithm
int
main(void)
{
    // Update default function before local initializations
    debug_mod_default_func = context;

    debug_mod dm[] = {
	{ NULL, NULL, "foo" },
	{ NULL, NULL, "bar" },
	{ NULL, NULL, "frobnicate" },
	{ NULL, NULL, "frog" },
	{ NULL, NULL, "fa" },
	{ NULL, NULL, "far" },
	{ NULL, NULL, "foofoo" },
	{ NULL, NULL, "farfalle" },
    };

    // Prefill the modules list with dummy entries
    for (debug_mod_index_t i = 0; i < ENTRIES(dm); ++i) {
	debug_mod_register(dm + i);
    }

    // Initialization must happen after all modules are registered
    search_init();
    // otherwise, the search table must be rebuilt after a change

    char c;
    while ((c = getchar()) != EOF) {
	putc(c, stdout);
	if (c == search_terminator) {	//end of search string
	    if (cursor) {	//all characters matched so far
		if (strlen(cursor) == 0) {	//complete match
		    printf("\tExact match %u: %s\n"
			   "---\n", mod_index, search_table[mod_index].id);
		    // Now reconfigure the module (example application)
		    mods[mod_index]->stream = stderr;
		} else {
		    printf("\tPartial match %u: %s\n"
			   "---\n", mod_index, search_table[mod_index].id);
		}
	    }
	    // Get ready for next search
	    reset_search();
	} else if (cursor) {		//still valid entries
	    incremental_search(c);
	    if (cursor) printf("\tCurrent match: %u=%s, rest:%s\n",
			       mod_index, search_table[mod_index].id, cursor);
	}
	if (! cursor) printf("\tNo match\n");
    }
    return 0;
}

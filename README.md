
libdebugmod - Debug Module Library
==================================

*libdebugmod* is a very small C library to easily manage debug output
in console or embedded applications.

Author: André Colomb <src@andre.colomb.de>

License
-------

Copyright (C) 2014  André Colomb

libdebugmod is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

libdebugmod is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program.  If not, see
<http://www.gnu.org/licenses/>.


Motivation
----------

Small programs, especially on embedded microcontrollers, often have
different needs for runtime debugging output than large-scale desktop
or server applications, where minimal runtime overhead or compiled
binary size is less of a concern.  Outputting debug messages during
the program's execution is often the most valuable source of
information for developers.

The traditional approach for controlling the level of debug verbosity
is to assign each possible message a severity level and then filter
out everything above a certain, runtime-configured level.  This level
can even be limited to a lower value for production builds with
smaller binary size and better runtime performance.

With *libdebugmod*, messages are instead enabled per translation unit
(called a code **module** in this context).  A translation unit is
typically one source file (.c or .cpp), or whatever the compiler
treats as the scope of a `static` variable.  This is coupled with a
decentralized architecture, where no central component needs to have a
list of all subsystems.  Referencing modules via hard-coded string
identifiers makes it possible to still easily control the verbosity
during runtime, for example through text commands or a console.

The overhead is kept minimal and configurable to allow small
executable sizes on embedded platforms.  Production builds are easily
stripped of _almost_ all debugging code (see "Limitations" below).  In
contrast to traditional approaches, where debug output code is hidden
by the preprocessor conditions, *libdebugmod* keeps the code visible
to the compiler.  Such debugging code is less likely to break during
restructuring and leads to compilation errors before releasing broken
debug code which the compiler has never had a chance to validate.


### Features ###
+ Standard C streams as I/O descriptors make logging to files or
  console easy.
+ Suitable for embedded systems, leaving the choice of output function
  to the user (`fprintf()` not mandatory).
+ No wrapper functions, allowing the compiler to e.g. validate printf
  format strings.
+ Zero runtime overhead for production builds.
+ Enable / disable debug output for individual modules at runtime.
+ Minimal runtime overhead when debugging is disabled.
+ Decentralized architecture for independent code module subsystems.
+ Function context is provided for each debug output.
+ Complete control over output prefixes via callback function.
+ Debugging code is always compiled and therefore consistent with
  other code changes (e.g. renamed variables).
+ Small implementation with optional features.


### Limitations ###
- Relies on optimizing compiler for zero-overhead production builds
- Slight binary size overhead for strings used as module identifier or
  function context (compiler-dependent)
- Not thread-safe


Installation
------------

### Requirements ###

There are no required dependencies for building the library.  Since
the code uses features from the C99 language standard, the compiler
must support this standard and it might have to be explicitly enabled
with a command line switch (e.g. `-std=c99`).

The source code and interfaces are well documented and prepared to
generate documentation with [Doxygen][1].  A handy configuration named
`Doxyfile` is included.

[1]: http://www.doxygen.org/ "Doxygen home page"


### Compilation ###

A simple Makefile is included under the `src` directory, compatible at
least with GNU make.  It contains rules to create the static library
and a test program demonstrating the API.  In addition, the `dump`
target outputs a disassembly of the test program to verify the library
overhead.

The number of debug modules which can be registered in a program is
limited during compilation of `debug_mod.c`.  This maximum can be
checked through the variable `debug_mod_max`.  To override it, define
the `DEBUG_MOD_MAX` macro to a numeric value during compilation.  The
default is **four** modules.

By default, compilation is done with the build machine's standard
tools like `cc` and `objdump`.  To test the library with this default
toolchain, use the `host` target:

	make host

As another example usage, the `avr` target provides a convenient way
to cross-build using the [avr-gcc toolchain][2] and then dump the
resulting disassembly for inspection:

	make avr

To use the library in your own code, there are two possible build
scenarios.  Either way, the `include` directory should be added to the
preprocessor include path for your project.

[2]: http://www.nongnu.org/avr-libc/ "AVR Libc, for use with avr-gcc"


#### Variant A: Using the Static Library ####

When building a native host application or if you just want a binary
file for use in projects, make the `lib` target in the provided
Makefile.  The standard variables from the environment or provided on
the command line can control the toolchain options (CC, LD, CPPFLAGS,
CFLAGS, LDFLAGS). Example:

	make -C libdebugmod/src/ CFLAGS="-g -O1" clean lib

Make sure to also configure optional library API features during this
step, as explained under the section "Runtime management API" below.
The built static library can then be regularly linked to your project
by providing the linker with a library path:

	cc -Ilibdebugmod/include/ -Llibdebugmod/src/ \
		myproject.c -ldebugmod -o myproject

Or by directly referencing the archive file:

	cc -Ilibdebugmod/include/ \
		myproject.c libdebugmod/src/libdebugmod.a -o myproject


#### Variant B: Direct Linking ####

If the library source resides within your project directory, it may be
easier to just include the source file for the API functions in your
build process.  This way, your toolchain configuration is always
consistent with the *libdebugmod* build.  Keep in mind that a C99 mode
compiler is required, though.

	cc -Ilibdebugmod/include/ \
		myproject.c libdebugmod/src/debug_mod.c -o myproject

If your project uses a Makefile, simply add the include path to your
`CPPFLAGS` variable, then add `libdebugmod/src/debug_mod.c` to the
source files.  Example Makefile excerpt:

	DEBUGMOD_DIR = libdebugmod
	CPPFLAGS += -I$(DEBUGMOD_DIR)/include
	myproject: myproject.c $(DEBUGMOD_DIR)/src/debug_mod.c


Basic API for Debug Output
--------------------------

To employ any *libdebugmod* functionality in one of your code modules,
you need to:

~~~~~~~~~~~~~{c}

	#define DEBUG_MOD_ENABLE	// This is optional
	#include <debug_mod.h>
	DEBUG_MOD_INIT("my module")
~~~~~~~~~~~~~

Without the `DEBUG_MOD_ENABLE` macro, the debug output code is still
visible to the compiler, but will be removed by the optimizer.  In
practice, it may be easier to define the `DEBUG_MOD_ENABLE` macro via
command line flags to the preprocessor during a debug build instead of
hard-coding it into each source file.

The `debug_mod.h` header file contains most of the API declaration and
provides the magic to separate debug channels for different
translation units.  It must be included once per translation unit.
Just look at its contents or the generated Doxygen documentation for
details.

The `DEBUG_MOD_INIT()` macro must also be used once per translation
unit, before any debug output.  It defines the static storage used to
identify and register the current code module.  Lazy people with a
good C preprocessor may use the magic constant `__FILE__` for the
module identifier:

	DEBUG_MOD_INIT(__FILE__)

Actual debug output is most conveniently generated with one of the
macros `DEBUGF()` and `DEBUGL()`.  They are designed to provide great
flexibility around the stream I/O functions from the C Standard
Library, namely `fprintf()`, `fputs()`, `fputc()`, etc.  Your average
debug message may look like this:

~~~~~~~~~~~~~{c}

	fprintf(stderr, "Houston, we have %d problems.", count);
	fputc('\n', stderr);
~~~~~~~~~~~~~

With *libdebugmod*, the calls should look like this:

~~~~~~~~~~~~~{c}

	DEBUGF(fprintf, "Houston, we have %d problems.", count);
	DEBUGL(fputc, '\n');
~~~~~~~~~~~~~

The `DEBUGF()` and `DEBUGL()` macros expand to a code block which

1. Is optimized away when `DEBUG_MOD_ENABLE` was not defined.
2. Evaluates a configured callback function to decide whether debug
   output is desired for the current module.
3. Calls the given output function with the configured stream as first
   or last argument, respectively.
4. Passes all remaining arguments to the output function verbatim.

The first two steps above can also be reused for code other than debug
output, for example to open a log file only when debugging is enabled.
The `DEBUG_CONDITION` macro expands to an `if (...)` statement with
the described behaviour.

The callback function serves two main purposes: a) Controlling whether
the specific debug statement should be executed by returning a
non-zero value and b) "preparing" the debug output, such as writing a
prefix (timestamp, etc.) to the output stream.  In addition to the
current module's debug configuration, it is passed a string describing
the calling context.  By default, the `__func__` magic preprocessor
macro is used, but can be overridden setting `DEBUG_MOD_CONTEXT` to
any expression evaluating to a string at runtime.


Runtime Management API
----------------------

The library allows to flexibly reconfigure each module at run-time.
The callback function deciding whether to execute the debug statements
can be exchanged, or entirely suppressed.  Switching the output stream
passed in `DEBUGF()` and `DEBUGL()` statements is also supported.

These configuration changes can be done from within each module for
its own settings, as well as through a module-external API to allow
for centralized management of debug facilities by the user.


### Self-Configuration Within Modules ###

Each module's own current output stream can be accessed directly
through `debug_mod_get_stream()` and modified using
`debug_mod_set_stream()`.  The default output stream at initialization
is always `stderr`.

Initially, each module's configuration is lazily initialized to use
the callback function address stored in `debug_mod_default_func`.  Set
the latter during early program initialization to provide a default
callback instead of disabling all debug modules.

Replacing the callback function is done via `debug_mod_set_func()`,
where a function pointer of type `debug_mod_f` must be passed in.
Setting the callback to `NULL` disables debugging output for this
module altogether, which is equivalent to `debug_mod_disable_self()`.

In some setups, a central module list may not be desired, but at some
point in time a complete enumeration of all registered modules is
needed.  However, the lazy initialization only happens upon the first
evaluation of the `DEBUG_CONDITION`, which implies all side-effects of
the respective callback function.  To avoid these, use
`debug_mod_register_self()` within each module, forcing registration
early in the program startup.  The `incremental_string_search.c` demo
program (explained below) includes such a usecase.


### External Reconfiguration API ###

Other modules can control the debug output experience using the API
provided in `debug_mod_control.h`.  To access the respective API, you
need to:

~~~~~~~~~~~~~{c}

	#define DEBUG_MOD_DYNAMIC	// This is optional (see below)
	#define DEBUG_MOD_SAVE		// This is optional (see below)
	#include <debug_mod_control.h>
~~~~~~~~~~~~~

This header file also pulls in the standard `debug_mod.h` header, so
the basic API for debug output can be used regularly after
initialization with `DEBUG_MOD_INIT()` (see above).  In addition the
function `debug_mod_register()` is declared and can be used to
configure a debug module in advance.

Each module's configuration is lazily initialized during the first
evaluation of the `DEBUG_CONDITION`, for example when first reaching a
`DEBUGF()` call.  At this time, the `debug_mod_init()` function
arranges two things:

  1. Make sure the module's identifier is registered in a central list
     using `debug_mod_register()`.
  2. Apply the `debug_mod_default_func` address (typically `NULL`),
     unless the module has already been registered, in which case the
     pre-recorded configuration is used.

This logic allows to enumerate modules and their individual settings
from a central place using `debug_mod_register()` during program
initialization, like in the following example.

~~~~~~~~~~~~~{c}

	// Example output preparation: Prefix with function context
	char cb_context(debug_mod* self, const char* context) {
		fputs(context, self->stream);
		fputs("(): ", self->stream);
		return 1;
	}

	int main(void) {
		// Enable debug output to stderr by default for any module
		debug_mod_default_func = cb_context;

		// Override a specific module's default stream
		debug_mod dm = { cb_context, stdout, "my module" };
		debug_mod_register(&dm);

		/* main program logic */

		return 0;
	}
~~~~~~~~~~~~~


### Dynamic Module Reconfiguration (optional) ###

This part of the control API is optional and only available if the
macro `DEBUG_MOD_DYNAMIC` was defined before including
`debug_mod_control.h`.  Remember to arrange this also when building
the static library, for example:

	make -C libdebugmod/src/ CPPFLAGS=-DDEBUG_MOD_DYNAMIC clean lib

Additional functions exposed are `debug_mod_update()` and
`debug_mod_disable()`.  In contrast to `debug_mod_register()`, they
can be used to alter modules' configurations anytime _after_ they have
been registered and used, from outside the affected module.

~~~~~~~~~~~~~{c}

	// Update callback and stream for all known modules
	debug_mod_update(NULL, cb_context, stdout);
	// Disable a single module's output again:
	debug_mod_disable("my module");
	// which is equivalent to:
	debug_mod_update("my module", NULL, NULL);
~~~~~~~~~~~~~

Specifying a `NULL` value for the module identifier applies the call
to all modules that have already been registered (explicitly or
automatically).


### Save and Restore Configuration (optional) ###

This part of the control API is optional and only available if the
macro `DEBUG_MOD_SAVE` was defined before including
`debug_mod_control.h`.  Remember to arrange this also when building
the static library, for example:

	make -C libdebugmod/src/ CPPFLAGS=-DDEBUG_MOD_SAVE clean lib

Additional functions exposed are:
- `debug_mod_list()` to iterate through all known modules
- `debug_mod_save()` to conveniently save the current modules'
  configuration to an array
- `debug_mod_restore()` to apply a previously stored configuration

~~~~~~~~~~~~~{c}

	debug_mod my_config[debug_mod_max];	// Note the maximum size
	debug_mod_save(my_config, sizeof(my_config) / sizeof(*my_config));
	// "my_config" may be copied to non-volatile memory now
	memcpy(wherever, my_config, sizeof(my_config);

	// ... during next run, load my_config again, then:
	debug_mod_restore(my_config, sizeof(my_config) / sizeof(*my_config));
~~~~~~~~~~~~~


Demo Programs
-------------

The source code includes a small demo program illustrating all of the
features named above.  Inside the `src/` directory, look at the files
`test_debug_mod.c` and `test_ext_module.c`.  The `Makefile` includes a
target to compile and run this demo using the host toolchain.  Note
that the preprocessor macros to enable features **must be defined
during this compilation step**.

	make -C libdebugmod/src/ \
		CPPFLAGS="-DDEBUG_MOD_ENABLE -DDEBUG_MOD_DYNAMIC -DDEBUG_MOD_SAVE" \
		clean test

Two compilation units are tested, using the preprocessor's special
`__FILE__` macro for the module identifier.  The `test_debug_mod.c`
module contains different example implementations for setup functions,
prepending the stream output with varying context information.

The test calls for API functions are all documented and use the
optional API if enabled.  The `main()` function demonstrates debug
output from an external module and its own, with various configuration
changes in between.

Another example lives in `test_incremental_search.c` and shows a more
sophisticated usage of the runtime management API.  It implements a
rather efficient incremental string search algorithm to match a
(partial) input against the list of registered debug modules,
exercising the `debug_mod_list()` function.  Use the following
`Makefile` target to run the example code with appropriate macro
definitions already included.

	make -C libdebugmod/src/ clean test-search

## Compilation with debug symbols

Pretty simple if using the build.sh script:
```
./build.sh -cgr
```
Should clean any previous builds, build in debug mode and run in GDB or the specified debugger.

To build manually:
```
cmake -DCMAKE_BUILD_TYPE=Debug
```
And then `make'.  Building using the script provided is easer and quicker, though.

## Headers

Here's some information about hidden headers that are kind of hard-to-explore.

csnippets/
	csnippets.h - This header includes some libc headers that are commonly used,
			also, it includes some of the headers in the same directory; those ones define
			some helpful macros, etc.
	attributes.h - This is included by the above header, it provides shortcuts for function/variable
			attributes, like, __attribute__((cold)) -> __cold and so on.
	error.h  - This defines some macros used to notify the user for errors/warnings and such things that occur
			during runtime.

internal/
	socket_compat.h - This header is not installed and should not be used by any header file that will be
				installed with the library, but only be included in source code or
				any other internal header.

Please note that csnippets/csnippets.h is included during compilation and not programmatically,
that's because it acts as a pre-compiled header.  The -include is passed to the compiler like so:
```sh
$compiler <options> -include csnippets/csnippets.h
```
This tells the compiler that we want that header included on top of the file that is being compiled.

## Coding Style

I use and prefer Linux style over all but may be switching to GNU coding standards since I'm starting to like them.

Use tabs of 8 width and braces in line not after.  If you feel the code looks like spaghetti, use astyle (or ./format_code.sh) to format it.

### Braces

#### If statements

Braces should be placed in the same line:
```c
if (x) {
	if (y)
		z();
}
```
Or:
```c
if (x)
	if (y)
		z();
```

Nested if statements can be confusing, so seperate each condition in a new line:
```c
if (foo) {
	bar();
	if (baz && whatever
	    (|| whatsoever
	      && lol) {
		... some interesting code ...	 
	}
```

#### Loops

Same as if statements:
```c
int i;

for (i = 0; i < whatever; i++)
	if (haha)
		ok();
```

#### Functions
Put braces after definition:
```c
void foo(void)
{
	... do some nasty stuff ...
}
```

It's okay to use the extern keyword to declare:
```c
extern void foo(void);
```
or:
```c
void foo(void);
```
is fine too.

Make the function file-scope whenever possible (if you're not going to use it in other files):
```c
static void bar(void)
{
	... lots of code ...
}
```

### Gotos

Feel free to use goto's whenever possible, they're helpful but try not to make it look like spaghetti
It's hard to follow goto statements so try to avoid them whenever you think you don't really need it.

Bad:
```c
static void baz(void)
{
	int i = 0;

	for (i = 0; i < n; i++)
		if (whatever[i] == some_interesting_value)
			goto done;

done:
	haha();
}
```
Fine:
```
static void blah(bool dontcare)
[
	int whatever;

	whatever = hello ();
	if (whatever && !dontcare)
		goto out;

	if (world() != dontcare)
		goto out;

	bleh (whatever, !dontcare);
out:
	/* Some nasty stuff before aborting.  */
	__builtin_abort ();
]
```

Comment code you feel confusing.

### Commits

Please [signoff](http://gerrit.googlecode.com/svn/documentation/2.0/user-signedoffby.html) your commits.

Use [detailed commit messages](http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html)

More to come.


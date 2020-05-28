# GDB

The GNU's debugger is a great tool to analyse binaries and here we will
learn the basics about it.

## Setting Up Environment

It is really important to make sure we are debugging the binary in an
environment similar to the one in which the actual binary will be
used. Of course, there are limitations to how similar such
environments can be.

For example, when debugging a program with the `setuid` bit set,
`gdb` will drop privileges. Aside from that, `gdb` has some default
settings we can tinker with to match our normal environment and
provide a better debugging experience:

* `disable-randomization`

* `environment variables`

* `flavor`

* `attaching`

You can change these settings everytie you enter `gdb` or write
them to `~/.gdbinit`, where `gdb` always execute commands from
when called.

### `disable-randomization`

By default, this setting is on, meaning that `gdb` won't use
ASLR (Address Space Layout Randomization) like our usual
environment. This means that our usual environment may not
repreduce the same results as `gdb`!

To change it and the other settings, we use the `set` command:

```
set disable-randomization off
```

Mind the double negatives!

### `environment variables`

Environment variables occupy space in the stack, so the addresses
used in a program can change between environments due to different
variables being used!

If we run `gdb` as `env - gdb`, it will be called inside a more
controlled environment in terms of variables, meaning that
only the variables that `gdb` set upon entry will be set.

If then we call `show env` inside `gdb`, we will see these
variables that `gdb` sets:

```
(gdb) show env
LINES=38
COLUMNS=150
```

Typing `unset env LINES` and `unsed env COLUMNS` will get rid
of these variables. If your shell by default use these variables,
don't worry! They will be there if set by your shell, but omitted
if only `gdb` sets them!

### `attaching`

`gdb` can attach itself to already running processes. This is
can help debug situations when your exploit only works in `gdb`
even though the environment variables are the same and
`disable-randomization` is off. You can either use the `attach`
command inside `gdb` and give the process PID as argument, or
type `gdb <binary name>`, press TAB (this will autocomplete
with the PID of a process running the binary) and then ENTER.

### `flavor`

`gdb` can show assembly code using two syntaxes: intel and AT&T.
By default, `gdb` uses AT&T but you may prefer intel. To change
to the desired flavor use:

```
set disassembly-flavor intel
```

or 

```
set disassembly-flavor att
```

And yes, this change is purely visual, to provide a better
debugging experience.

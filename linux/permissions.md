# Permissions in Linux

## File Permissions

Everything in Linux is a file. Every file in Linux have permissions
which say who is allowed to write, read and execute it. Not only that,
each file can be set to run as a different user.

We can see the permissions on a file by running ``` ls -l <filename>```

To change the permissions on a file, the **chmod** command can be used
in the terminal.

```
chmod 764 <filename>
```

### Read, Write and Execute

The first digit set what the owner of the file can do. \
The second digit set what the users in the owner's group can do. \
The third digit set what anybody can do.

First Digit | Second Digit | Third Digit
------------|--------------|-------------
Owner | Group | everyone

Each digit can have a value between 0 and 7. The idea is that every bit
in the digit's binary form correspond to write, read or execute permissions.

First bit (4) | Second bit (2) | Third bit (1)
--------------|----------------|--------------
Read | Write | Execute

In our example of chmod above, we are setting so that the owner can
read, write and execute, the owner's group can read and write, and anybody
can read.

> 7 = 4 + 2 + 1 (all bits are set, owner can do anything) \
> 6 = 4 + 2 (read and write bits are set, that's what the group can do) \
> 1 = 1 (only the read bit is set, anyone can read the file)

### Special bits

We can also call chmod using four digits as argument.

```
chmod 4760 <filename>
```

In this case, we are not only changing the read, write and execute
permissions but also the file special bits. The syntax is the following:

First Digit | Second Digit | Third Digit | Fourth Digit
------------|--------------|-------------|--------------
Special bits | Read | Write | Execute

Just like with the other digits, the special bits digit can have a value
from 0 to 7 and each bit have an special meaning.

The first bit is the Setuid bit. When set it means that whoever runs the
file will run it as the owner. Has no effect in directories.\
The second bit is the Segid bit. When set it means that whoever runs the
file will run it as if it was part of the owner's group. If the file is
a directory, all its files will be affected recursively.\
The Third bit is the sticky bit. It only has some effect when the file
is an directory. When it is set in a directory, it means that all the files
in the directory can only be modified by their owners. A simple use case of
this is the `/tmp` directory.

So, in the example above, using 4760 as `chmod` argument means that whoever
runs this file will run it as the owner. This is can be really handy for 
an attacker if the owner happens to be root and they can find a
vulnerability in the executable.

## UID, GUID and friends

So, when we say a use can "run a file as another user", what do we really
mean? It means we are changing some user IDs. In Linux each user has an
ID associated with it: the UID (User ID)

### RUID, EUID and saved user ID

Each process in Linux keeps track of the RUID, EUID, saved user ID and its
counterparts for the groups IDs: RGID, EGID and saved group ID.

So, what are these?

**RUID and RGID** are the real user and group ids. **RUID** is the ID of
the user that called the program and **RGID** is the ID of this user group.

**EUID and EGID** are the effective user and group ids. These are the
values that the Setuid and Setgid bits change. They are the IDs of the user
and the group that the file is running as.

**saved user and group ID** at the beginning of each program the EUID is
equal to the saved user ID (same thing happen to the group counterparts).
So if the EUID is changed (for example, by calling `setresuid`) we can
change it back by reading the saved user ID and writing it to EUID.

## Attacks

If a file has the Setuid bit set, we can try to attack to get a shell
running as the file owner. In this case there is some observations to
take into account.

1. `bash` requires both RUID and EUID to be set to the file owner's to work
and sometimes it drops privileges anyway, if possible, use `sh` instead
(only require EUID).

2. if `sh` is bash version 2, privileges will also be dropped. So, at
the end of the day, always aim for `execl` (require only EUID)

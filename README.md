# Simple shell made for cs345 course -Computer Science Department University Of Crete 

# The shell: 

    1.First display prompt and create array[struct commands(argument array is inside the struct)] by going through the whole input line.
    2.Then for every single command check for non child commands cd - '=' - exit - fg and execute them without forking.
    3.If not found check if the sequence of upcoming commands are pipes and call mypipe if so.
    4.Otherwise fork and exec in child

This is repeated.

# Shell support list

## Signals:
- ctrl s
- ctrl q
- ctrl z
- fg

## Commands:
- Regular single commands (ending with or without ';')
- Multiple commands in a single parse separated with ';' (ending with or without ';')

## Variables:
- Global variables that can be accesed within the shell and subshell

## Pipes:
- Single pipe commands
- Multipiped commands

# Prerequisites
- Make
- gcc

# How to run
Run **./run.sh** in the main directory.

# Comments about the code:
1. signals were very hard to find info for, hard to make and they dont work 100%.
2. Parser was challenging. The whole thing was made without using strtok or strtok_r.




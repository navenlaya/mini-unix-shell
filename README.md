# mini-unix-shell

## Features

- Interactive prompt (mysh> )
- Command parsing with whitespace tokenization
- Process execution via fork() / execvp() / waitpid()
- Clean exit with exit command or Ctrl+D

## Build

```bash
make
```

Or without make:

```bash
gcc -Wall -Wextra -pedantic -std=c99 -o mysh main.c parse.c execute.c
```

## Usage

```
$ ./mysh
mysh> echo hello world
hello world
mysh> ls -l
...
mysh> exit
```

#
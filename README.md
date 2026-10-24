
# OS Lab 3: Mini Shell

**Course:** CC373: Operating Systems  
**Institution:** Alexandria University, Faculty of Engineering  
**Language:** C++

## Overview
This project is a custom Unix-like mini shell written in C++. It parses user input and executes system commands, replicating the core functionalities of standard shells like `bash` or `sh`. 

## Features Implemented
* **Command Execution:** Spawns child processes using `fork()` and executes commands using `execvp()`.
* **File Redirection:** Supports input (`<`), output (`>`), append (`>>`), error (`2>`), and combined output/error (`>>&`) redirection using `dup2()`.
* **Pipes:** Chains multiple commands together using `pipe()`, feeding the standard output of one command into the standard input of the next.
* **Background Processing:** Supports running commands in the background using `&`. Includes a `SIGCHLD` signal handler to safely reap zombie processes and log termination times to `shell.log`.
* **Built-in Commands:** 
  * `cd [dir]`: Changes the current working directory (defaults to `HOME` if no directory is provided).
  * `exit`: Gracefully terminates the shell.
* **Signal Handling:** Ignores `Ctrl-C` (`SIGINT`) in the main shell prompt to prevent accidental closures, while allowing foreground child processes to be interrupted.
* **✨ BONUS: Wildcarding:** Implements file expansion for `*` and `?` wildcard characters using the `glob()` library.

## File Structure
* `command.cc` / `command.h`: The core logic of the shell, handling the command table structure, process creation, piping, redirection, and built-in commands.
* `tokenizer.cc` / `tokenizer.h`: Parses raw string input into executable tokens (arguments, pipes, redirect symbols, etc.).
* `Makefile`: Automates the compilation and linking process.
* `shell.log`: Automatically generated log file tracking the exit timestamps of background child processes.

## How to Build and Run

1. Open your terminal in the project directory.
2. Compile the project using the provided Makefile:
   ```bash
   make
   ```
3. Run the shell:
   ```bash
   ./myshell
   ```

## Example Usage
Once inside the `myshell>` prompt, you can test the following scenarios:

**Basic Execution & Arguments:**
```bash
ls -al
```

**Output Redirection:**
```bash
ls -al > output.txt
```

**Piping:**
```bash
ls -al | grep command
```

**Background Processing:**
```bash
sleep 5 &
```

**Wildcard Bonus:**
```bash
echo *.cc
```

**Exit Shell:**
```bash
exit
```

## Clean Up
To remove the compiled object files and the executable, run:
```bash
make clean
```

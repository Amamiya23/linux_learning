# AGENTS.md - Linux FileIO Learning Project

## Project Overview
This is a C++17 project for learning Linux file I/O operations. It contains multiple standalone executables organized into stages.

## Build Commands

### Full Build
```bash
cd /home/cat/Learning/fileio/build
cmake ..
make
```

### Build Single Target
```bash
cd /home/cat/Learning/fileio/build
make <target_name>
```
Available targets: `file_copy`, `read_append`, `file_seeker`, `file_stats`, `dir_list`, `dir_walker`, `file_lock`, `memory_map`, `logger_demo`

### Run Built Binaries
```bash
/home/cat/Learning/fileio/build/bin/<target_name> [args]
```

### Clean Build
```bash
cd /home/cat/Learning/fileio/build
make clean
rm -rf *
cmake ..
```

## Code Style Guidelines

### C++ Standard
- Use C++17 features (project requires C++17)
- Use `std::string` instead of C-style strings
- Use `size_t` for sizes and ssize_t for signed size values

### Naming Conventions
- Classes: `PascalCase` (e.g., `Logger`, `FileHandler`)
- Methods/Functions: `snake_case` or `camelCase` (e.g., `get_current_logfile()`, `init()`)
- Member variables: `snake_case_` with trailing underscore (e.g., `log_dir_`, `current_fd_`)
- Constants: `SCREAMING_SNAKE_CASE` (e.g., `BUFFER_SIZE`, `MAX_FILES`)
- Enums: `PascalCase` for enum class values (e.g., `LogLevel::DEBUG`)

### Error Handling
- Use `errno` and `strerror(errno)` for system call errors
- Use `EXIT_FAILURE` (1) and `EXIT_SUCCESS` (0) for exit codes
- For recoverable errors, return `false` and log the error
- Use `std::cerr` for error output
- Always close file descriptors in error paths

### Imports Organization
1. C++ standard library headers (`<iostream>`, `<fstream>`, `<string>`, etc.)
2. C headers (`<fcntl.h>`, `<unistd.h>`, `<sys/stat.h>`, etc.)
3. Custom headers (project headers like `"logger.h"`)

Use `#include <cxxtyle>` form for C library wrappers (e.g., `#include <cstring>` not `#include <string.h>`).

### Formatting
- Indent with 4 spaces
- Opening brace on same line for functions/classes
- One declaration per line
- Maximum line length: ~100 characters (soft limit)

### Memory and Resource Management
- Always close file descriptors immediately after use
- Use `std::lock_guard<std::mutex>` for mutex locking
- Use RAII principles where possible
- For file locks, pair `flock(fd, LOCK_EX)` with `flock(fd, LOCK_UN)`

### Class Design
- Use singleton pattern with `static Logger& instance()` where appropriate
- Delete copy constructor and copy assignment operator for resource-holding classes
- Mark destructors `virtual` only when class is intended for inheritance
- Private members after public members in class definition

### System Calls Pattern
```cpp
int fd = open(path, flags, mode);
if (fd == -1) {
    // handle error with strerror(errno)
    return false/error_code;
}
// use fd
close(fd);
```

### Multithreading
- Use `std::mutex` and `std::lock_guard` for thread safety
- Check `current_fd_ != -1` before operations
- Reset to -1 after closing: `close(current_fd_); current_fd_ = -1;`

## Project Structure
```
fileio/
├── 01_basic/         # Basic file operations (copy, read/append)
├── 02_position/      # File positioning and stats
├── 03_directory/     # Directory operations
├── 04_advanced/      # File locking, memory mapping
├── 05_projects/      # Logger project
└── CMakeLists.txt    # Build configuration
```

## Notes
- No formal test framework is currently set up
- Each executable is a standalone learning example
- Use `access()` to check file existence before operations
- Use `struct stat` and `fstat()` for file metadata

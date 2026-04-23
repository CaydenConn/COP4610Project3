# COP4610 Project 3: FAT32 File System

## Project Description
This project implements a user-space utility to interpret and interact with a FAT32 file system image. The program provides a custom shell interface that supports navigating the directory structure, reading file contents, and modifying the file system (creating, deleting, writing, and moving files/directories). This project involves parsing the boot sector, traversing the File Allocation Table (FAT), managing directory entries, and implementing standard file operations.

## Student Information
* **Name:** Cayden Conn
* **FSUID:** cbc22b

* **Name:** Chase Blancher
* **FSUID:** cgb22

* **Name:** Giovanni Giannone
* **FSUID:** gg22e

## Getting Started
1. Ensure you have the `fat32.img` file in the root directory of the project.
2. Execute `make` in the root directory to compile the project.
3. Run the shell by executing `./filesys fat32.img`.
4. You can also run the automated test suite using `python3 test_filesys.py`.

## Supported Features / Components
* **Boot Sector Parsing:** `info` command to display volume information.
* **Directory Navigation:** `cd`, `ls` (lists files and directories, including long file names).
* **File Creation & Deletion:** `mkdir`, `creat`, `rm`, `rmdir`.
* **File Operations:** `open`, `close`, `lsof` (list open files), `lseek`, `read`, `write`.
* **File Management:** `mv` (moving files and directories).

## File Structure & Listing
* `Makefile`: Compiles the C source files into the `filesys` executable.
* `fat32.img`: The FAT32 image file used for testing.
* `test_filesys.py`: Automated Python test script for the implemented features.
* **`include/`**
  * `fat32.h`, `shell.h`, `dir.h`, `lexer.h`, `commands.h`: Core data structures and function prototypes.
  * `cmd_*.h`: Header files for individual commands.
* **`src/`**
  * `main.c`: Entry point of the application.
  * `fat32.c`: Logic for interacting with the FAT32 volume (boot sector, clusters).
  * `dir.c`: Logic for reading and modifying directory entries and long file names.
  * `lexer.c`, `shell.c`: Shell prompt and command parsing logic.
  * `cmd_*.c`: Implementations for each shell command (`cd`, `ls`, `read`, `write`, `rm`, etc.).

## Division of Labor

**Cayden Conn**
* Part 3: Create (`mkdir`, `creat`)
* Part 6: Delete (`rm`, `rmdir`)

| Date       | Work Completed / Notes                                      |
|------------|-------------------------------------------------------------|
| 2026-04-17 | Implemented Part 3 (`mkdir` and `creat`) with FAT cluster allocation. |
| 2026-04-21 | Implemented Part 6 (`rm` and `rmdir`) and fixed a bug in `dir.h`. |

**Chase Blancher**
* Part 4: Read (`open`, `close`, `lsof`, `lseek`, `read`)
* Part 5: Update (`write`, `mv`)
* Python Test Suite (`test_filesys.py`)

| Date       | Work Completed / Notes                                      |
|------------|-------------------------------------------------------------|
| 2026-04-20 | Tested parts 1-3 and finalized project integration logic.   |
| 2026-04-20 | Implemented Parts 4 and 5 operations and the py test file.  |
| 2026-04-23 | Finalized read/write edge cases and updated automated tests. |

**Giovanni Giannone**
* Part 1: Mounting the Image File (`info`, `exit`)
* Part 2: Navigation (`cd`, `ls`)

| Date       | Work Completed / Notes                                      |
|------------|-------------------------------------------------------------|
| 2026-04-16 | Initialized project repository and integrated `fat32.img`.  |
| 2026-04-16 | Implemented Part 1 and 2 (`info`, `cd`, `ls`) with Makefile.|
| 2026-04-22 | Fixed directory name bugs and made commands case-sensitive. |

## Meetings

| Date       | Attendees                                      | Topics Discussed                     | Outcomes / Decisions                     |
|------------|------------------------------------------------|--------------------------------------|------------------------------------------|
| 2026-04-09 | Giovanni Giannone, Chase Blancher, Cayden Conn | Initial project setup & FAT structure| Created basic project scaffolding.       |
| 2026-04-16 | Giovanni Giannone, Chase Blancher, Cayden Conn | Modifying FAT tables & directory reqs| Decided on how to track open file state. |
| 2026-04-23 | Giovanni Giannone, Chase Blancher, Cayden Conn | Final bug fixing & testing           | Validated all features against tests.    |

## Known Issues
* None at this time.

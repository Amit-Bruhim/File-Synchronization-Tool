![Banner](images/banner.png)  

![C](https://img.shields.io/badge/language-C-blue)

# File Synchronization Tool 🗂️  

🔧 Command-line utility in C for efficient file synchronization between source and destination folders 

## Table of Contents  

1. [About](#about)  
2. [Features](#features)  
3. [Requirements](#requirements)  
4. [Installation](#installation)  
5. [Usage](#usage)  

---

## About

The **File Synchronization Tool** is a lightweight C program that synchronizes files between a source and destination directory.  
It keeps both folders consistent by comparing and updating their contents automatically.  
The project demonstrates system-level programming concepts such as **process creation (fork)** and **program execution (exec)** to perform file operations efficiently.

---

## Features  

- Synchronizes files between source and destination directories  
- Detects and handles new files  
- Detects and handles files that are newer in source or destination  
- Skips files that are identical  
- Copies updated or new files automatically  

---

## Requirements

- A Linux, macOS, or Windows system with WSL for running C programs  
- GCC (GNU Compiler Collection) or another C compiler  
- Git (to clone the repository)  
 
---

## Installation

Follow these steps to set up the project locally:

---

### 1. Clone the repository
```bash
git clone https://github.com/Amit-Bruhim/File-Synchronization-Tool.git
```

### 2. Compile the program
```bash
gcc -o file_sync file_sync.c
```

### 3. Run the program

Make sure to provide the required arguments: a source directory and a destination directory.

```bash
./file_sync <source_directory> <destination_directory>
```

---

## Usage

When you run the program, it will first print an opening message showing the current working directory and the source/destination directories being synchronized:  

![Opening message screenshot](images/opening_message.png)

Then, it will process each file individually, printing messages depending on the file's status.  
For demonstration, we will use a classic example of two folders containing one file of each type:  
new, identical, and files that differ in modification time between source and destination.


![File processing screenshot](images/file_processing.png)

When synchronization completes, a final message will be displayed:  

![Completion message screenshot](images/completion_message.png)

> **Note:** Issues such as missing directories or missing command-line arguments will result in an error message.

For user convenience, you can create example folders with prepared files by running the following script:

```bash
#!/bin/bash

mkdir -p source_dir dest_dir

# source files
echo "new file content" > source_dir/new_file.txt
echo "identical content" > source_dir/same_file.txt
echo "in folder: source, new is in src" > source_dir/source_newer.txt
echo "in folder: source, new is in dest" > source_dir/dest_newer.txt

# dest files
echo "identical content" > dest_dir/same_file.txt
echo "in folder: dest, new is in src" > dest_dir/source_newer.txt
echo "in folder: dest, new is in dest" > dest_dir/dest_newer.txt

# set modification times
touch -t 202510201310 source_dir/source_newer.txt
touch -t 202510201307 dest_dir/source_newer.txt
touch -t 202510201300 source_dir/dest_newer.txt
touch -t 202510201307 dest_dir/dest_newer.txt
```

After running this script, you can test the program:

```bash
./file_sync source_dir dest_dir
```

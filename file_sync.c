// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

// global vars
int EXPECTED_ARGC = 2;
int FILES_AMOUNT = 0;
int status = 0;
int MAX_STRING = 1024;
char INVALID_ARGC_STRING[] = "\033[31mUsage: file_sync <source_directory> <destination_directory>\033[0m";
char CURRENT_DIR_FORMAT[] = "\033[32mCurrent working directory: \033[0m%s";
char NON_EXISTING_SRC_DIR_FORMAT[] = "\033[31mError: Source directory '%s' does not exist.\033[0m";
char NON_EXISTING_DEST_DIR_FORMAT[] = "\033[32mCreated destination directory '%s'.\033[0m";
char FORK_ERR_STRING[] = "\033[31mfork failed\033[0m";
char GETCWD_ERR_STRING[] = "\033[31mgetcwd failed\033[0m";
char CHDIR_ERR_STRING[] = "\033[31mchdir failed\033[0m";
char EXEC_ERR_STRING[] = "\033[31mexec failed\033[0m";
char WAIT_ERR_STRING[] = "\033[31mwait failed\033[0m";
char NEW_FILE_STRING[] = "\033[32mNew file found: \033[0m%s";
char SAME_FILE_STRING[] = "\033[32mFile \033[0m%s\033[32m is identical. Skipping...\033[0m";
char NEWER_FILE_STRING[] = "\033[32mFile \033[0m%s\033[32m is newer in destination. Skipping...\033[0m";
char OLDER_FILE_STRING[] = "\033[32mFile \033[0m%s\033[32m is newer in source. Updating...\033[0m";
char GOODBYE_STRING[] = "\n\033[32mSynchronization complete.\033[0m\n";
char SYNC_STRING[] = "\033[32mSynchronizing from \033[0m%s\033[32m to \033[0m%s\n";
char COPIED_STRING[] = "\033[32mCopied:\033[0m %s/%s \033[32m->\033[0m %s/%s";

// declaration
void handle_diffrent_files(char *file, char *argv[]);
void fork_and_exec(char *path, char *argv[], int isDiff);
void validate_args(int argc, char *argv[]);
void validate_argc(int argc);
void print_current_dir();
void validate_src_dir(char *argv[]);
void validate_dest_dir(char *argv[]);
void run(char *argv[]);
char **sort_files(char *folder);
void create_and_copy(char *file, char *argv[]);
void print_sync_message(char *argv[]);

// function that validate the input
void validate_args(int argc, char *argv[])
{
   // validate argc
   validate_argc(argc);
   // print the current directory
   print_current_dir();
   // print the sync message
   print_sync_message(argv);
   // validate src dir exist
   validate_src_dir(argv);
   // validate dest dir exist
   validate_dest_dir(argv);
}

// print the sync message
void print_sync_message(char *argv[])
{
   // set vars
   char src_path[MAX_STRING];
   char dest_path[MAX_STRING];

   realpath(argv[1], src_path);
   realpath(argv[2], dest_path);

   // print message
   printf(SYNC_STRING, src_path, dest_path);
   printf("\n");
}

// function that validate the src dir
void validate_src_dir(char *argv[])
{
   // tey to open the src dir
   DIR *pDir;
   pDir = opendir(argv[1]);
   // if the dir isn't exists, print a message and exit
   if (pDir == NULL)
   {
      printf(NON_EXISTING_SRC_DIR_FORMAT, argv[1]);
      exit(1);
   }
}

// function that validate the dest dir
void validate_dest_dir(char *argv[])
{
   // try to open the dest dir
   DIR *pDir;
   pDir = opendir(argv[2]);
   // if the dir doesn't exist
   if (pDir == NULL)
   {
      // print a message
      printf(NON_EXISTING_DEST_DIR_FORMAT, argv[2]);
      printf("\n");

      // create the directory
      char *args[] = {"mkdir", argv[2], NULL};
      fork_and_exec("/bin/mkdir", args, 0);
   }
}

// function that prints the current directory
void print_current_dir()
{
   char cwd[PATH_MAX];
   if (getcwd(cwd, sizeof(cwd)) != NULL)
   {
      printf(CURRENT_DIR_FORMAT, cwd);
      printf("\n");
   }
   else
   {
      perror(GETCWD_ERR_STRING);
      exit(1);
   }
}

// function that makes sure we got exactly 2 arguments
void validate_argc(int argc)
{
   if (argc != EXPECTED_ARGC + 1)
   {
      print_current_dir();
      printf("%s", INVALID_ARGC_STRING);
      exit(1);
   }
}

// start the sync
void run(char *argv[])
{
   // sort the files
   char **files_names = sort_files(argv[1]);
   // iterate over the files in src dir
   for (int i = 0; i < FILES_AMOUNT; i++)
   {

      if (chdir(argv[2]) != 0)
      {
         perror("Failed to change directory to destination!");
         exit(1);
      }
      // if the file is not in dest
      if (access(files_names[i], F_OK) == -1)
      {
         // create and copy the file from src to dest
         create_and_copy(files_names[i], argv);
      }
      // if the file is in dest
      else
      {
         // create and copy the new file
         char src_path[MAX_STRING];
         char dest_path[MAX_STRING];

         // create the paths
         sprintf(src_path, "%s/%s", argv[1], files_names[i]);
         sprintf(dest_path, "%s/%s", argv[2], files_names[i]);

         // compare
         chdir("..");
         char *args[] = {"diff", "-q", src_path, dest_path, NULL};
         fork_and_exec("/bin/diff", args, 1);
         // same
         if (!status)
         {
            printf(SAME_FILE_STRING, files_names[i]);
            printf("\n");
         }
         else
         {
            handle_diffrent_files(files_names[i], argv);
         }
      }
   }
}

// function that handle diffrent files
void handle_diffrent_files(char *file, char *argv[])
{
   // get the time data
   struct stat src_date;
   struct stat dest_date;

   // create and copy the new file
   char src_path[MAX_STRING];
   char dest_path[MAX_STRING];

   // create the paths
   sprintf(src_path, "%s/%s", argv[1], file);
   sprintf(dest_path, "%s/%s", argv[2], file);

   stat(src_path, &src_date);
   stat(dest_path, &dest_date);

   // the src is newer
   if (difftime(src_date.st_mtime, dest_date.st_mtime) > 0)
   {
      printf(OLDER_FILE_STRING, file);
      printf("\n");
      // copy
      char *args[] = {"cp", src_path, dest_path, NULL};
      fork_and_exec("/bin/cp", args, 0);

      // set vars
      char src_path2[MAX_STRING];
      char dest_path2[MAX_STRING];

      realpath(argv[1], src_path2);
      realpath(argv[2], dest_path2);

      printf(COPIED_STRING, src_path2, file, dest_path2, file);
      printf("\n");
   }
   // the dest is newer or same
   else
   {
      printf(NEWER_FILE_STRING, file);
      printf("\n");
   }
}


// create and copy the file from src to dest
void create_and_copy(char *file, char *argv[])
{
   // print a message
   printf(NEW_FILE_STRING, file);
   printf("\n");
   // create and copy the new file
   char src_path[MAX_STRING];
   char dest_path[MAX_STRING];

   // create the paths
   sprintf(src_path, "%s/%s", argv[1], file);
   sprintf(dest_path, "%s/%s", argv[2], file);

   // copy
   char *args[] = {"cp", src_path, dest_path, NULL};
   chdir("..");
   fork_and_exec("/bin/cp", args, 0);

   // set vars
   char src_path2[MAX_STRING];
   char dest_path2[MAX_STRING];

   realpath(argv[1], src_path2);
   realpath(argv[2], dest_path2);

   printf(COPIED_STRING, src_path2, file, dest_path2, file);
   printf("\n");
}

// function that creates new procces
void fork_and_exec(char *path, char *argv[], int isDiff)
{
   // create new procces
   pid_t pid = fork();
   // handle fork error
   if (pid < 0)
   {
      // error in fork
      perror(FORK_ERR_STRING);
      exit(1);
   }
   else
   {
      // in the son create a new dir
      if (pid == 0)
      {
         if (isDiff)
         {
            int null_fd = open("/dev/null", O_WRONLY);
            dup2(null_fd, STDOUT_FILENO);
            close(null_fd);
         }
         if (execv(path, argv) < 0)
         {
            perror(EXEC_ERR_STRING);
            exit(1);
         }
      }
      // in the parent wait
      else
      {
         wait(&status);
      }
   }
}

// function that get the files and sort them
char **sort_files(char *folder)
{
   // set vars
   char **files_names = NULL;
   DIR *dir = opendir(folder);
   struct dirent *entity;
   // read all the files
   entity = readdir(dir);
   while (entity != NULL)
   {
      // continue only if it's a file
      if (entity->d_type == 8)
      {
         // increment the files amount, allocate memory and set the file name
         FILES_AMOUNT++;
         files_names = realloc(files_names, sizeof(char *) * FILES_AMOUNT);
         files_names[FILES_AMOUNT - 1] = malloc(strlen(entity->d_name) + 1);
         strcpy(files_names[FILES_AMOUNT - 1], entity->d_name);
      }
      entity = readdir(dir);
   }
   closedir(dir);

   // sort
   char *tmp;
   for (int i = 0; i < FILES_AMOUNT; i++)
   {
      for (int j = i + 1; j < FILES_AMOUNT; j++)
      {
         if (strcmp(files_names[i], files_names[j]) < 0)
         {
            tmp = files_names[i];
            files_names[i] = files_names[j];
            files_names[j] = tmp;
         }
      }
   }

   // reverse
   for (int i = 0; i < FILES_AMOUNT / 2; i++)
   {
      tmp = files_names[i];
      files_names[i] = files_names[FILES_AMOUNT - 1 - i];
      files_names[FILES_AMOUNT - 1 - i] = tmp;
   }

   return files_names;
}

// code starts here
int main(int argc, char *argv[])
{
   // validate the args
   validate_args(argc, argv);
   // start the sync
   run(argv);
   printf("%s", GOODBYE_STRING);
   return 0;
}
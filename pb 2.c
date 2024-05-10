#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#define BUFSIZE 4096

/*

struct stat
{
  dev_t st_dev;   // device
  ino_t st_ino;   // inode
  umode_t st_mode;   // protection
  nlink_t st_nlink;   // number of hard links
  uid_t st_uid;   // user ID of owner
  gid_t st_gid;   // group ID of owner
  dev_t st_rdev;   // device type (if inode device)
  off_t st_size;   // total size, in bytes
  unsigned long st_blksize;   // blocksize for filesystem I/O
  unsigned long st_blocks;   // number of blocks allocated
  time_t st_atime;   // time of last access
  time_t st_mtime;   // time of last modification
  time_t st_ctime;   // time of last change
};

*/

long int _log10(long int x)
{
  int digits=0;
  
  if (x == 0)
    return 1;
  while (x != 0)
    {
      digits++;
      x /= 10;
    }
  return digits;
}

void rezolvare(char *input, char *path, char *output_path)
{
  // printf("rezolvare: %d: %s\n", i, input);
  
  int fd1, fd2, inod_digits=0, size_digits=0, mtime_digits=0;
  char buf[BUFSIZE], *human_readable_date, virtual_path[256], inod_tostring[256], output_path2[256];
  struct stat filedata, dirdata;

  strcpy(virtual_path, path);
  strcat(virtual_path, "/");
  strcat(virtual_path, input);
  
  if( (fd1 = open(virtual_path, O_RDONLY)) < 0)
    {
      stat(virtual_path, &filedata);
      printf("Permission bits: %03o (%s)\n", filedata.st_mode&511, input);
      if ((filedata.st_mode&511) == 0)
	{
	  printf("File with no permissions: %s\n\n", input);
	  // TODO create process...
	  return;
	}
      else
	{
	  printf("File does not exists in the current context\n");
	  exit(3);
	}
    }

  fstat(fd1, &filedata);
  
  inod_digits = _log10(filedata.st_ino);
  size_digits = _log10(filedata.st_size);
  mtime_digits = _log10(filedata.st_mtime);

  human_readable_date = ctime(&filedata.st_mtime);
  human_readable_date[strlen(human_readable_date)-1] = 0;
// sprintf(buf, "   INOD %ld\n\nFile Name: %s\nLast Modification: %s (st_mtime: %ld)\nFile Size: %ld bytes\n\n\n", filedata.st_ino, input, human_readable_date, filedata.st_mtime, filedata.st_size);
  sprintf(buf, "   INOD %ld\n\nFile Name: %s\nFull Path: %s\nLast Modification: %s (st_mtime: %ld)\nPermission bits: %03o\nFile Size: %ld bytes\n\n\n", filedata.st_ino, input, virtual_path, human_readable_date, filedata.st_mtime, filedata.st_mode&511, filedata.st_size);
  // printf("%s\n", buf);

  sprintf(inod_tostring, "%ld", filedata.st_ino);
  strcpy(output_path2, output_path);
  strcat(output_path2, "/");
  strcat(output_path2, inod_tostring);
  strcat(output_path2, ".txt");
  
  if( (fd2 = open(output_path2, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0)
    {
      printf("Error creating instance in output folder for a directory\n");
      exit(4);
    }
  write(fd2, buf, 106+inod_digits+strlen(input)+strlen(virtual_path)+strlen(ctime(&filedata.st_mtime))+mtime_digits+size_digits);

  if (S_ISDIR(filedata.st_mode))
    {
      DIR *dir;
      struct dirent *entry;
      char create_dir_path[256];
      
      strcpy(create_dir_path, output_path);
      strcat(create_dir_path, "/.");
      strcat(create_dir_path, input);

      if (stat(create_dir_path, &dirdata) != 0)
	{
	  if (mkdir(create_dir_path, 0777) == -1)
	    {
	      printf("Error creating directory for accurate in the output file\n");
	      exit(5);
	    }
	}
      
      if ((dir = opendir(virtual_path)) == NULL)
	{
	  printf("Error opening directory\n");
	  exit(6);
	}
      while ((entry = readdir(dir)) != NULL)
	{
	  if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
	    {
	      rezolvare(entry->d_name, virtual_path, create_dir_path);
	    }
	}
    }

  close(fd1);
  close(fd2);
  
  return;
}

/*
void rezolvare2(char *input, char *output, char *path)
{
  int fd1, fd2;
  char input_path[256], output_path[256], inode[256];
  struct stat filedata;

  strcpy(input_path, path);
  strcat(input_path, "/");
  strcat(input_path, input);

  strcpy(output_path, path);
  strcat(output_path, "/");
  strcat(output_path, output);
  
  if ((fd1 = open(input_path, O_RDONLY)) < 0)
    {
      printf("Error opening input path\n");
      exit(2);
    }
  fstat(fd1, &filedata);
  snprintf(inode, "%ld", filedata.st_ino);

  DIR *dir;
  struct dirent *entry;
  if ((dir = opendir(output_path)) == NULL)
    {
      printf("Error opening output path\n");
      exit(3);
    }
  while ((entry = readdir(dir)) != NULL)
    {
      if (strcmp(entry->d_name, inode) == 0)
	{
	  // am gasit inode-ul fisierului in fisierul de iesire
	}
    }
  
  
}
*/

int main(int argc, char *argv[])
{
  int i, status, output_index;
  char path[256], output_path[256];
  pid_t pid[21];   // vector de pid-uri ale proceselor create
  struct stat dirdata;

  rename("/home/onusike/Desktop/Bananas", "/home/onusike/Desktop/Banana");
  
  pid[0] = 1;
  for (i=0; i<=20; i++)
    {
      pid[i] = 1;
    }
  
  for (i=1; i<argc-1; i++)
    {
      if (argv[i][0] == '-')
	{
	  if (argv[i][1] == 'o')
	    {
	      output_index = i+1;
	    }
	}
    }

  strcpy(path, realpath(".", NULL));
  strcpy(output_path, realpath(".", NULL));
  strcat(output_path, "/");
  strcat(output_path, argv[output_index]);

  if (stat(output_path, &dirdata) != 0)
    {
      if (mkdir(output_path, 0777) == -1)
	{
	  printf("Error creating the output directory\n");
	  exit(1);
	}
    }
  
  for (i=1; i<argc; i++)
    {
      if (argv[i][0] != '-' && i != output_index)
	{
	  if (pid[i-1] != 0)   // pid[i-1] este pid-ul ultimului proces creat, daca nu e 0 ne aflam in procesul parinte
	    {
	      if ( (pid[i]=fork()) < 0 )   // creare procesul fiu cu numarul i
		{
		  printf("Error creating child process");
		  exit(2);
		}
	      if (pid[i] == 0)   // in procesul fiu se monitorizeaza modificarile asupra fisierului i transmis in linia de comanda
		{
		  // cod fiu
		  rezolvare(argv[i], path, output_path);
		  exit(0);
		}
	    }
	}
    }
  for (i=1; i<argc; i++)
    {
      if (argv[i][0] != '-' && i != output_index)
	{
	  waitpid(pid[i], &status, 0);   // waitpid pentru fiecare proces fiu
	  if(WIFEXITED(status))
	    printf("\nProcesul cu PID-ul %d s-a încheiat cu codul %d (%s)\n", pid[i], WEXITSTATUS(status), argv[i]);
	  else
	    printf("\nAnormalitate in cadrul procesului cu PID-ul %d (%s)\n", pid[i], argv[i]);

	}
    }
  return 0;
}

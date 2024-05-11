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

void rezolvare(char *input, char *path, char *output_path, int *cnt, int modified_path, int *pfd)
{
  // printf("rezolvare: %d: %s\n", i, input);
  int fd1, fd2, fd3, status, size_digits=0, mtime_digits=0, i, n;
  char buf[2*BUFSIZE], *human_readable_date, virtual_path[BUFSIZE], inod_tostring[BUFSIZE], mtime_tostring[BUFSIZE], output_path2[BUFSIZE], bash_path[BUFSIZE], prev_data[6][BUFSIZE], current_dir_path[BUFSIZE], modified_dir_path[BUFSIZE], lines[BUFSIZE], *line;
  pid_t pid;
  struct stat filedata, dirdata;

  close(pfd[0]);
  
  strcpy(virtual_path, path);
  strcat(virtual_path, "/");
  strcat(virtual_path, input);
  
  if( (fd1 = open(virtual_path, O_RDONLY)) < 0)
    {
      stat(virtual_path, &filedata);
      // printf("Permission bits: %03o (%s)\n", filedata.st_mode&511, input);

      if ((filedata.st_mode&511) == 0)
	{
	  // printf("File with no permissions: %s\n\n", input);

	  if ( (pid=fork()) < 0 )   // creare proces pentru analiza sintactica
	    {
	      printf("Error creating child process");
	      exit(255);
	    }
	  if (pid == 0)   // procesul de analizare a fisierului
	    {
	      // cod fiu
	      strcpy(bash_path, realpath(".", NULL));
	      strcat(bash_path, "/malicious_check.sh");
	      
	      execlp("/bin/bash", "bash", bash_path, virtual_path, NULL);

	      printf("\nEroare la execlp\n");
	      exit(255);
	    }
	  
	  waitpid(pid, &status, 0);

	  if (WIFEXITED(status))
	    {
	      if (WEXITSTATUS(status) != 0)
		{
		  // SEND TO PARENT VIA PIPE

		  strcat(virtual_path, "\n\0");
		  write(pfd[1], virtual_path, strlen(virtual_path));
		  (*cnt)++;
		  return;
		}
	      else
		{
		  write(pfd[1], "SAFE\n", 5);
		}
	    }
	  else
	    {
	      printf("\nAnormalitate in cadrul procesului cu PID-ul %d (malicious_check.sh for %s)\n", pid, input);
	    }
	}
      else
	{
	  printf("File does not exists in the current context\n");
	  exit(255);
	}
    }
  else
    {
      fstat(fd1, &filedata);
    }
  
  // inod_digits = _log10(filedata.st_ino);
  size_digits = _log10(filedata.st_size);
  mtime_digits = _log10(filedata.st_mtime);

  human_readable_date = ctime(&filedata.st_mtime);
  human_readable_date[strlen(human_readable_date)-1] = 0;
  sprintf(buf, "File Name: %s\nFull Path: %s\nLast Modification: %ld (%s)\nPermission bits: %03o\nFile Size: %ld bytes\n\n\n", input, virtual_path, filedata.st_mtime, human_readable_date,  filedata.st_mode&511, filedata.st_size);

  sprintf(inod_tostring, "%ld", filedata.st_ino);
  strcpy(output_path2, output_path);
  strcat(output_path2, "/");
  strcat(output_path2, inod_tostring);
  strcat(output_path2, ".txt");

  if( (fd3 = open(output_path2, O_RDONLY)) < 0)
    {
      // printf("Error opening file named after inode\n");

      if( (fd2 = open(output_path2, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0)
	{
	  printf("Error creating instance in output folder for a directory\n");
	  exit(255);
	}
      write(fd2, buf, 86+strlen(input)+strlen(virtual_path)+strlen(ctime(&filedata.st_mtime))+mtime_digits+size_digits);
    }
  else
    {
      while ( (n = read(fd3, &lines, BUFSIZE ) > 0) )
	{
	  for (i=0, line=strtok(lines, "\n"); i<=4; i++, line=strtok(0, "\n"))
	    {
	      strcpy(prev_data[i], strchr(line, ':')+2);
	    }
	}
      prev_data[2][10] = 0;
      sprintf(mtime_tostring, "%ld", filedata.st_mtime);
      if ( strcmp(input, prev_data[0]) != 0 || ( strcmp(prev_data[2], mtime_tostring) != 0 && prev_data[2][1] != ' ' ) || modified_path == 1)
	{
	  if( (fd2 = open(output_path2, O_WRONLY | O_TRUNC)) < 0)
	    {
	      printf("Error opening instance in output folder for a directory\n");
	      exit(255);
	    }
	  write(fd2, buf, 86+strlen(input)+strlen(virtual_path)+strlen(ctime(&filedata.st_mtime))+mtime_digits+size_digits);
	  if ( S_ISDIR(filedata.st_mode) && strcmp(input, prev_data[0]) != 0 )
	    {
	      strcpy(current_dir_path, output_path);
	      strcat(current_dir_path, "/.");
	      strcat(current_dir_path, prev_data[0]);

	      strcpy(modified_dir_path, output_path);
	      strcat(modified_dir_path, "/.");
	      strcat(modified_dir_path, input);
	      
	      rename(current_dir_path, modified_dir_path);
	      modified_path = 1;
	    }
	}
    }
  
  

  if (S_ISDIR(filedata.st_mode))
    {
      DIR *dir;
      struct dirent *entry;
      char create_dir_path[BUFSIZE];
      
      strcpy(create_dir_path, output_path);
      strcat(create_dir_path, "/.");
      strcat(create_dir_path, input);

      if (stat(create_dir_path, &dirdata) != 0)
	{
	  if (mkdir(create_dir_path, 0777) == -1)
	    {
	      printf("Error creating directory for accurate representation in the output file\n");
	      exit(255);
	    }
	}
      
      if ((dir = opendir(virtual_path)) == NULL)
	{
	  printf("Error opening directory\n");
	  exit(255);
	}
      while ((entry = readdir(dir)) != NULL)
	{
	  if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
	    {
	      rezolvare(entry->d_name, virtual_path, create_dir_path, cnt, modified_path, pfd);
	    }
	}
    }

  close(fd1);
  close(fd2);
  close(fd3);
  
  return;
}

int main(int argc, char *argv[])
{
  int i, j, n, status, output_index=0, safe_index=0;
  char path[BUFSIZE], output_path[BUFSIZE], safe_path[BUFSIZE], lines[BUFSIZE], *line, filename[BUFSIZE], new_safe_location[BUFSIZE];
  pid_t pid[21];   // vector de pid-uri ale proceselor create
  struct stat dirdata;
  
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
	  else if (argv[i][1] == 's')
	    {
	      safe_index = i+1;
	    }
	}
    }

  strcpy(path, realpath(".", NULL));
  
  strcpy(output_path, realpath(".", NULL));
  strcat(output_path, "/");
  strcat(output_path, argv[output_index]);

  strcpy(safe_path, realpath(".", NULL));
  strcat(safe_path, "/");
  strcat(safe_path, argv[safe_index]);

  if (stat(output_path, &dirdata) != 0)
    {
      if (mkdir(output_path, 0777) == -1)
	{
	  printf("Error creating the output directory\n");
	  exit(255);
	}
    }

  int pfd[2];
  if ( pipe(pfd) < 0 )
    {
      printf("Eroare la crearea pipe-ului\n");
      exit(255);
    }
  
  for (i=1; i<argc; i++)
    {
      if (argv[i][0] != '-' && i != output_index && i != safe_index)
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
		  int cnt=0;
		  rezolvare(argv[i], path, output_path, &cnt, 0, pfd);
		  close(pfd[1]);
		  exit(cnt);
		}
	    }
	}
    }

  close(pfd[1]);
  
  for (i=1, j=1; i<argc; i++)
    {
      if (argv[i][0] != '-' && i != output_index && i != safe_index)
	{
	  waitpid(pid[i], &status, 0);   // waitpid pentru fiecare proces fiu
	  if (WIFEXITED(status))
	    printf("\nProcesul copil %d s-a incheiat cu PID-ul %d si cu %d fisiere cu potential periculos\n", j++, pid[i], WEXITSTATUS(status));
	  else
	    printf("\nAnormalitate in cadrul procesului copil %d cu PID-ul %d\n", j++, pid[i]);
	}
    }

  while ( (n = read(pfd[0], &lines, BUFSIZE ) > 0) )
    {
      for (line=strtok(lines, "\n"); line!=0; line=strtok(0, "\n"))
	{
	  if ( strcmp(line, "SAFE") != 0)
	    {
	      if (stat(safe_path, &dirdata) != 0)
		{
		  if (mkdir(safe_path, 0777) == -1)
		    {
		      printf("Error creating safe directory\n");
		      exit(9);
		    }
		}
	      strcpy(filename, strrchr(line, '/')+1);
	  
	      strcpy(new_safe_location, safe_path);
	      strcat(new_safe_location, "/");
	      strcat(new_safe_location, filename);
	  
	      rename(line, new_safe_location);
	      }
	}
    }

  close(pfd[0]);
  
  return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
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

void rezolvare(int i, char *input, char *output)
{
  // printf("rezolvare: %d: %s\n", i, input);
  
  int fd1, fd2, inod_digits=0, size_digits=0, mtime_digits=0;
  long int aux;
  char buf[BUFSIZE], *human_readable_date;
  struct stat filedata;
  
  if( (fd1 = open(input, O_RDONLY)) < 0)
    {
      printf("Error opening input file\n");
      exit(2);
    }

  fstat(fd1, &filedata);
  if( (fd2 = open(output, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) < 0)
    {
      printf("Error creating destination file\n");
      exit(3);
    }

  aux = filedata.st_ino;
  while (aux != 0)
    {
      inod_digits++;
      aux /= 10;
    }

  aux = filedata.st_size;
  if (aux == 0)
    size_digits++;
  while (aux != 0)
    {
      size_digits++;
      aux /= 10;
    }

  aux = filedata.st_mtime;
  while (aux != 0)
    {
      mtime_digits++;
      aux /= 10;
    }
  // write(fd2, buf, strlen(input)+8+digits);


  
  sprintf(buf, "   INOD %ld\n\n", filedata.st_ino);
  printf("   INOD %ld\n\n", filedata.st_ino);
  write(fd2, buf, 10+inod_digits);

  sprintf(buf, "File Name: %s\na", input);
  printf("File Name: %s\n", input);
  write(fd2, buf, 12+strlen(input));

  human_readable_date = ctime(&filedata.st_mtime);
  human_readable_date[strlen(human_readable_date)-1] = 0;
  sprintf(buf, "Last Modification: %s (st_mtime: %ld)\n", human_readable_date, filedata.st_mtime);
  printf("Last Modification: %s (st_mtime: %ld)\n", human_readable_date, filedata.st_mtime);
  write(fd2, buf, 32+strlen(ctime(&filedata.st_mtime))+mtime_digits);

  sprintf(buf, "File Size: %ld bytes\n\n\n", filedata.st_size);
  printf("File Size: %ld bytes\n\n\n", filedata.st_size);
  write(fd2, buf, 20+size_digits);
  

  /*
  human_readable_date = ctime(&filedata.st_mtime);
  human_readable_date[strlen(human_readable_date)-1] = 0;
 
  sprintf(buf, "   INOD %ld\n\nFile Name: %s\nLast Modification: %s (st_mtime: %ld)\nFile Size: %ld bytes\n\n\n", filedata.st_ino, input, human_readable_date, filedata.st_mtime, filedata.st_size);
  printf("%s\n", buf);
  write(fd1, buf, 74+inod_digits+strlen(input)+strlen(ctime(&filedata.st_mtime))+mtime_digits+size_digits);
  */

  /*
  while((n = read(fd1, buf, BUFSIZE)) > 0)
    {
      if(write(fd2, buf, n) < 0)
	{
	  printf("Error writing to file\n");
	  exit(4);
	}
    }
  */
  
  close(fd1);
  close(fd2);

  return;
}

int main(int argc, char *argv[])
{
  int i, status;
  pid_t pid[11];   // vector de pid-uri ale proceselor create
  pid[0] = 1;
  // printf("%d\n", argc);
  for (i=1; i<argc-1; i++)
    {
      if (pid[i-1] != 0)   // pid[i-1] este pid-ul ultimului proces creat, daca nu e 0 ne aflam in procesul parinte
	{
	  if ( (pid[i]=fork()) < 0 )   // creare procesul fiu cu numarul i
	    {
	      perror("Eroare");
	      exit(0);
	    }
	  if (pid[i] == 0)   // in procesul fiu se monitorizeaza modificarile asupra fisierului i transmis in linia de comanda
	    {
	      // cod fiu
	      rezolvare(i, argv[i], argv[argc-1]);
	      exit(1);
	    }
	}
    }
  for (i=2; i<argc-1; i++)
    {
      waitpid(pid[i], &status, 0);   // waitpid pentru fiecare proces fiu
      if(WIFEXITED(status))
	printf("\nProcesul cu PID-ul %d s-a încheiat cu codul %d\n", pid[i], WEXITSTATUS(status));
      else
	printf("\nAnormalitate in cadrul procesului cu PID-ul %d\n", pid[i]);
    }
  return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEBUG

#define K          1024
#define SLASH       '/'

char **split(char *src);
void freelist(char **args);
/*
int main(int argc, char *argv[])
{
  char s[K];
  char **args;
  int i;

  if(argc < 2)
    return 1;
  strcpy(s, argv[1]);

  args = split(s);
  if(!args)
    return 1;

  for(i=0;args[i];i++)
    printf("%2d %2d  %s\n", i,  strlen(args[i]), args[i]);
  freelist(args);
}
*/
char **split(char *path)
{
  int i, nc;           // number of path components
  char *p;             // loops through the src string
  char *src;           // our copy of path
  char **tmp;          // the answer

  if(*path != SLASH)    // absolute path must start with /
    return 0;

  src = malloc(strlen(path) + 1);
  strcpy(src,path);

  nc = 1;
  for(p = src+1; *p ; p++)  // count components, already verified we have at least one
    if(*p == SLASH && *(p+1) != SLASH)
      nc++;

#ifdef DEBUG
  fprintf(stderr,"found %d components\n", nc);
#endif

  tmp = malloc((nc+1) * sizeof(char *));
  tmp[0] = src + 1;                     // +1 to avoid the 1st '/'
  for(i = 1 , p = src + 1; *p ; p++)
    if(*p == SLASH){
      *p = 0;
      if(*(p+1) != SLASH)
        tmp[i++] = p+1;
    }
  tmp[i] = 0;
  if(i != nc){
    fprintf(stderr,"WTF: %d != %d in %s\n", i, nc, src);
    exit(1);
  }
  return tmp;
}
void freelist(char **args)
{
  if(args){
    free(--args[0]);
    free(args);
  }
}

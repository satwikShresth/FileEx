#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dict.h"

struct employee {
    char name[128];
    int age;
    int salary;
    int years;
};
struct words {
    char word[128];
};

void load_database_words(struct dict *H, const char *filename)
{
  FILE *fp;
  int ret;
  int uniqueWords = 0;
  int totalWords;
  
  struct words* item = malloc(sizeof(*item));
  for(totalWords=0;fscanf(fp,"%s",item->word) != EOF;totalWords++)
  {
    int length = 0;
    for(int count =0;(*(item->word+count)!= '\0');count++)
      {
        if (isalpha(*(item->word+count)))
        {
          *(item->word+length) = tolower(*(item->word+count));
          length++;
        }
      }
    *(item->word+length)='\0';
    if (isalpha(*item->word) != 0)
    {
      if(dict_peek(H,item->word) != NULL)
      {
        uniqueWords++;
        ;
      }
    }
    else{
      totalWords--;
    }
  }
  fclose(fp);
  printf("Total Words: %d\n",totalWords);
  printf("Unique Words: %d\n",uniqueWords);
  printf("Hash Size: %d\n",H->nbins);
}

int load_database(struct dict *D, const char *filename)
{
    FILE *fp;
    int ret;

    struct employee *item;

    if ( !(fp = fopen(filename, "r")) )
        return 0;

    while (1) {
        item = malloc(sizeof(*item));
        ret = fscanf(fp, "%[^,], %d, %d, %d\n",
            item->name, &item->age, &item->salary, &item->years);

        if (ret == EOF) {
            free(item);
            break;
        }

        dict_insert(D, item->name, item);
    }

    fclose(fp);

    return 1;
}


void deleter(void *item)
{
    free(item);
}


int main(int argc, char **argv)
{
  int ret;
  struct dict team;
  if (strcmp(argv[1],"CSV")){
    struct employee *item;
  }
  else if (strcmp(argv[1],"Words")){
    struct words *item;
  }
  else{
    fprintf(stderr, "Unable to process the input.\n");
  }

  dict_init(&team, deleter);

  ret = load_database(&team, argv[2]);

  if (!ret) {
      fprintf(stderr, "Failed to open database.\n");
      return EXIT_FAILURE;
  }

  printf("\nLoad Factor: %0.2f\n", dict_loadfactor(&team));

  printf("Destroying Dictionary and exiting...\n");
  dict_destroy(&team);

  return 0;
}
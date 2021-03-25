
#include <stdio.h>
#include <string.h>
//#include <sys/types.h>
#include <dirent.h>
/* filter struct for file search */

struct FILTER{
char *name;
long long size ;
__uint32_t mode;
__uint16_t number_of_link;
}FILTER = {.mode=0,.size=0,.number_of_link=0};
typedef struct FILTER filter;




/* print file list_files_r*/
void list_files_r(char *target_path, int depth);


int main(int argc, char **argv)
{

    // Directory path to list files
    char path[100];

    // Input path from user
    printf("Enter path to list files: ");

    scanf("%s", path);
    printf("%s\n",path);

    list_files_r(path, 0);

    return 0;
}



void list_files_r(char *target_path, int depth)
{

    int i;
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(target_path);

    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {   printf("|");
            for (i=0; i< (depth + 1) * 2; i++)
            {
                    printf("%c", '-');

            }

            printf("%s\n", dp->d_name);

            strcpy(path, target_path);
            strcat(path, "/");
            strcat(path, dp->d_name);
            list_files_r(path, depth + 1);
        }
    }
    
    closedir(dir);
}

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>

#include <ctype.h>
#include <stdlib.h>
/* filter struct for file search */
struct FILTER {
  char *name;
  long long size;
  mode_t perms;
  mode_t type;
  __uint16_t number_of_link;
} default_filters = {
    .perms = 0, .type = 0, .size = 0, .number_of_link = 0, .name = "\0"};

typedef struct FILTER filter;

struct path_node {
  char *path;
  int depth;
  struct path_node *next;
  struct path_node *child;
};
typedef struct path_node path_node;
int is_equal(char *, char *);
void traverseTree(path_node *root);
path_node *addSibling(path_node *n, path_node *data);

path_node *addChild(path_node *n, path_node *data);

/* mask filter permissions type by given type*/
int mask_mode_by_perms(const char *perm, mode_t *mode);

int mask_mode_by_type(char type, mode_t *mode);

/* print file list_files_r*/
path_node *list_files_r(char *target_path, int depth, filter filter,
                        char *name);

// void print_perms(mode_t perms);
char *to_lover(char *);

int main(int argc, char *argv[]) {
  filter filter = default_filters;
  int c;
  char *root_path = "\0";

  if (argc < 3) {
    fprintf(stderr, "invalid arguments\n");
    exit(EXIT_FAILURE);
  }

  opterr = 0;
  while ((c = getopt(argc, argv, "w:f:b:t:p:l:")) != -1) {
    switch (c) {
    case 'f':
      (filter.name = (char *)malloc((strlen(optarg) + 1) * sizeof(char))) !=
              NULL
          ? strcpy(filter.name, optarg)
          : exit(EXIT_FAILURE);
      break;
    case 'b':
      if ((filter.size = atoi(optarg)) == 0) {
        fprintf(stderr, "invalid size: %s", optarg);
        exit(EXIT_FAILURE);
      }
      break;
    case 'l':
      if (0 == (filter.number_of_link = atoi(optarg))) {
        fprintf(stderr, "invalid link number: %s", optarg);
        exit(EXIT_FAILURE);
      }
      break;
    case 'w':
      (root_path = (char *)malloc((strlen(optarg) + 1) * sizeof(char))) != NULL
          ? strcpy(root_path, optarg)
          : exit(EXIT_FAILURE);

      break;
    case 'p':
      if (-1 == mask_mode_by_perms(optarg, &filter.perms)) {
        fprintf(stderr, "invalid permissions\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 't':
      if (-1 == mask_mode_by_type(optarg[0], &filter.type)) {
        fprintf(stderr, "invalid type\n");
        exit(EXIT_FAILURE);
      }
      break;
    case '?':
    default:
      fprintf(stderr, "invalid option: -%c\n", optopt);
      exit(EXIT_FAILURE);
    }
  }
  if (*root_path == '\0')
    fprintf(stderr, "invalid arguments: no root_path\n");

  path_node *tree = list_files_r(root_path, 0, filter, root_path);
  //    free(filter.name);
  traverseTree(tree);
  free(root_path);
  return 0;
}

// void print_perms(mode_t perms) {
//  printf((perms & S_IRUSR) ? "r" : "-");
//  printf((perms & S_IWUSR) ? "w" : "-");
//  printf((perms & S_IXUSR) ? "x" : "-");
//  printf((perms & S_IRGRP) ? "r" : "-");
//  printf((perms & S_IWGRP) ? "w" : "-");
//  printf((perms & S_IXGRP) ? "x" : "-");
//  printf((perms & S_IROTH) ? "r" : "-");
//  printf((perms & S_IWOTH) ? "w" : "-");
//  printf((perms & S_IXOTH) ? "x" : "-");
//}

path_node *list_files_r(char *target_path, int depth, filter filter,
                        char *name) {
  struct stat st;
  int i = 0;
  char path[1000];
  struct dirent *dp;

  path_node *node = (path_node *)malloc(sizeof(path_node));
  node->path = (char *)malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(node->path, name);
  node->next = NULL;
  node->child = NULL;
  node->depth = depth;
  errno = 0;
  if (stat(target_path, &st) < 0) {

    perror(target_path);
    return NULL;
    //    exit(EXIT_FAILURE);
  }
  //  printf("f:%3o\n", filter.type);
  if ((filter.name[0] == '\0' ||
       is_equal(to_lover(filter.name), to_lover(node->path)) == 0) &&
      (filter.size == 0 || filter.size == st.st_size) &&
      (filter.type == 0 || filter.type == (S_IFMT & st.st_mode)) &&
      (filter.perms == 0 || filter.perms == (0777 & st.st_mode)) &&
      (filter.number_of_link == 0 || filter.number_of_link == st.st_nlink)) {
    i = 1;
  }

  path_node *p_node = node;
  DIR *dir = opendir(target_path);

  if (dir) {
    while ((dp = readdir(dir)) != NULL) {

      if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
        strcpy(path, target_path);
        strcat(path, "/");
        strcat(path, dp->d_name);
        errno = 0;
        if (stat(target_path, &st) < 0) {

          perror(target_path);
          return NULL;
          //    exit(EXIT_FAILURE);
        }

        if (!(filter.name[0] != '\0' &&
              strcmp(to_lover(filter.name), to_lover(dp->d_name)) != 0) &&
            !(filter.size != 0 && filter.size != st.st_size) &&
            !(filter.type != 0 && filter.type != (S_IFMT & st.st_mode)) &&
            !(filter.perms != 0 && filter.perms != (0777 & st.st_mode)) &&
            !(filter.number_of_link != 0 &&
              filter.number_of_link != st.st_nlink)) {

          path_node *tmpnode = (path_node *)malloc(sizeof(path_node));
          tmpnode->path =
              (char *)malloc(sizeof(char) * (strlen(dp->d_name) + 1));
          strcpy(tmpnode->path, dp->d_name);
          tmpnode->depth = depth + 1;
          tmpnode->next = NULL;
          addChild(node, tmpnode);
          i = 1;
        }

        if ((addChild(node, list_files_r(path, depth + 1, filter,
                                         dp->d_name))) != NULL) {
          i = 1;
        };
      }
    }
    closedir(dir);
  }
  if (!i) {
    free(p_node->path);
    path_node *tmp = p_node;

    while (tmp != NULL) {
      p_node = tmp;
      tmp = tmp->next;
      free(p_node->next);
      free(p_node);
    }
  }
  return i != 0 ? p_node : NULL;
}

int mask_mode_by_perms(const char *perms, mode_t *mode) {
  int c = 0;
  if (strlen(perms) != 9) {
    return -1;
  }
  perms[0] == 'r' ? *mode |= S_IRUSR : perms[0] == '-' ?: (c = -1);
  perms[1] == 'w' ? *mode |= S_IWUSR : perms[1] == '-' ?: (c = -1);
  perms[2] == 'x' ? *mode |= S_IXUSR : perms[2] == '-' ?: (c = -1);
  perms[3] == 'r' ? *mode |= S_IRGRP : perms[3] == '-' ?: (c = -1);
  perms[4] == 'w' ? *mode |= S_IWGRP : perms[4] == '-' ?: (c = -1);
  perms[5] == 'x' ? *mode |= S_IXGRP : perms[5] == '-' ?: (c = -1);
  perms[6] == 'r' ? *mode |= S_IROTH : perms[6] == '-' ?: (c = -1);
  perms[7] == 'w' ? *mode |= S_IWOTH : perms[7] == '-' ?: (c = -1);
  perms[8] == 'x' ? *mode |= S_IXOTH : perms[8] == '-' ?: (c = -1);
  return c;
}

int mask_mode_by_type(const char type, mode_t *mode) {

  switch (type) {
  case 'f':
    *mode = S_IFREG;
    return S_IFREG;
  case 'd':
    *mode = S_IFDIR;
    return S_IFDIR;

  case 's':
    *mode = S_IFSOCK;
    return S_IFSOCK;
  case 'b':
    *mode = S_IFBLK;
    return S_IFBLK;
  case 'c':
    *mode = S_IFCHR;
    return S_IFCHR;
  case 'p':
    *mode = S_IFIFO;
    return S_IFIFO;
  case 'l':
    *mode = S_IFLNK;
    return S_IFLNK;
  default:
    return -1;
  }
}
char *to_lover(char *p) {

  char *t = p;
  for (; *p; ++p)
    *p = (char)tolower(*p);
  return t;
}

// Adds a sibling to a list with starting with n
path_node *addSibling(path_node *n, path_node *data) {
  if (n == NULL)
    return NULL;

  while (n->next != NULL)
    n = n->next;

  return (n->next = data);
}

// Add child Node to a Node
path_node *addChild(path_node *n, path_node *data) {
  if (n == NULL)
    return NULL;

  // Check if child list is not empty.
  if (n->child != NULL)
    return addSibling(n->child, data);
  else
    return (n->child = data);
}
void traverseTree(path_node *root) {
  if (root == NULL) {
    return;
  } else
    while (root != NULL) {
      printf("|");
      for (int i = 0; ((root->depth)) > i; i++) {
        printf("--");
      }
      printf("%s\n", root->path);
      if (root->child != NULL)
        traverseTree(root->child);
      path_node *tmp = root;
      root = root->next;
      free(tmp->path);
      free(tmp);
    }
}
int is_equal(char *x, char *y) {
  int l1, l2;
  int i, j;
  l1 = strlen(x);
  l2 = strlen(y);

  for (i = l1 - 1, j = l2 - 1; i >= 0 && j >= 0;) {
    if (x[i] != y[j]) {
      if (x[i] == '+') {
        if (x[i - 1] == y[j]) {
          j--;
        }

        else if (x[i - 1] == y[j + 1]) {
          i--;
          i--;
        }
      } else
        return -1;
    } else {
      i--;
      j--;
    }
  }
  if (j < 1 && i < 1) {
    return 0;
  } else
    return -1;
}

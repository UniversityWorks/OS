#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

#define DB_FILE ".important_files.db"
#define BUFF_SIZE 4096

char* getStoragePath() {
    static char path[BUFF_SIZE];
    char *home = getenv("HOME");
    if (home == NULL) {
        printf("ERROR: can't find home directory\n");
        return NULL;
    }
    sprintf(path, "%s/%s", home, DB_FILE);
    return path;
}

char* getAbsPath(const char *p) {
    static char result[BUFF_SIZE];
    if (realpath(p, result) == NULL) {
        return NULL;
    }
    return result;
}

int checkFileExists(const char *p) {
    struct stat info;
    if (stat(p, &info) == 0 && S_ISREG(info.st_mode)) {
        return 1;
    }
    return 0;
}

int checkMarked(const char *path) {
    char *db = getStoragePath();
    if (db == NULL) {
        return 0;
    }
    
    FILE *file = fopen(db, "r");
    if (file == NULL) {
        return 0;
    }
    
    char buffer[BUFF_SIZE * 2];
    int result = 0;
    
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        int len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        if (strcmp(buffer, path) == 0) {
            result = 1;
            break;
        }
    }
    
    fclose(file);
    return result;
}

int doMark(const char *p) {
    if (checkFileExists(p) == 0) {
        printf("ERROR: file '%s' does not exist\n", p);
        return 1;
    }
    
    char *abs = getAbsPath(p);
    if (abs == NULL) {
        printf("ERROR: cannot get absolute path\n");
        return 1;
    }
    
    if (checkMarked(abs) == 1) {
        return 0;
    }
    
    char *db = getStoragePath();
    if (db == NULL) {
        return 1;
    }
    
    FILE *file = fopen(db, "a");
    if (file == NULL) {
        printf("ERROR: cannot open database file\n");
        return 1;
    }
    
    fprintf(file, "%s\n", abs);
    fclose(file);
    
    return 0;
}

int doUnmark(const char *p) {
    if (checkFileExists(p) == 0) {
        printf("ERROR: file '%s' does not exist\n", p);
        return 1;
    }
    
    char *abs = getAbsPath(p);
    if (abs == NULL) {
        printf("ERROR: cannot get absolute path\n");
        return 1;
    }
    
    char *db = getStoragePath();
    if (db == NULL) {
        return 1;
    }
    
    FILE *file = fopen(db, "r");
    if (file == NULL) {
        return 0;
    }
    
    char tmpPath[BUFF_SIZE];
    sprintf(tmpPath, "%s.temp", db);
    
    FILE *tmpFile = fopen(tmpPath, "w");
    if (tmpFile == NULL) {
        fclose(file);
        printf("ERROR: cannot create temp file\n");
        return 1;
    }
    
    char buffer[BUFF_SIZE * 2];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        int len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        if (strcmp(buffer, abs) != 0) {
            fprintf(tmpFile, "%s\n", buffer);
        }
    }
    
    fclose(file);
    fclose(tmpFile);
    
    rename(tmpPath, db);
    return 0;
}

int checkMatch(const char *fname, const char *extension, const char *substring) {
    if (extension != NULL) {
        const char *dot = strrchr(fname, '.');
        if (dot == NULL || strcmp(dot + 1, extension) != 0) {
            return 0;
        }
    }
    
    if (substring != NULL) {
        if (strstr(fname, substring) == NULL) {
            return 0;
        }
    }
    
    return 1;
}

void searchInDir(const char *dirPath, const char *extension, const char *substring) {
    DIR *directory = opendir(dirPath);
    if (directory == NULL) {
        return;
    }
    
    struct dirent *item;
    while ((item = readdir(directory)) != NULL) {
        if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0) {
            continue;
        }
        
        char fullPath[BUFF_SIZE];
        sprintf(fullPath, "%s/%s", dirPath, item->d_name);
        
        struct stat info;
        if (stat(fullPath, &info) != 0) {
            continue;
        }
        
        if (S_ISDIR(info.st_mode)) {
            searchInDir(fullPath, extension, substring);
        } else if (S_ISREG(info.st_mode)) {
            char *abs = getAbsPath(fullPath);
            if (abs != NULL && checkMarked(abs) == 1 && checkMatch(item->d_name, extension, substring) == 1) {
                printf("%s\n", abs);
            }
        }
    }
    
    closedir(directory);
}

int doFind(const char *directory, const char *extension, const char *substring) {
    char searchPath[BUFF_SIZE];
    
    if (directory != NULL) {
        if (realpath(directory, searchPath) == NULL) {
            printf("ERROR: directory '%s' does not exist\n", directory);
            return 1;
        }
    } else {
        if (getcwd(searchPath, sizeof(searchPath)) == NULL) {
            printf("ERROR: cannot get current directory\n");
            return 1;
        }
    }
    
    searchInDir(searchPath, extension, substring);
    return 0;
}

void showHelp() {
    printf("Usage:\n");
    printf("  important mark <file>\n");
    printf("  important unmark <file>\n");
    printf("  important find [options]\n");
    printf("\nOptions:\n");
    printf("  --dir <directory>\n");
    printf("  --ext <extension>\n");
    printf("  --name-contains <text>\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        showHelp();
        return 1;
    }
    
    if (strcmp(argv[1], "mark") == 0) {
        if (argc != 3) {
            printf("ERROR: file path required\n");
            return 1;
        }
        return doMark(argv[2]);
    }
    
    if (strcmp(argv[1], "unmark") == 0) {
        if (argc != 3) {
            printf("ERROR: file path required\n");
            return 1;
        }
        return doUnmark(argv[2]);
    }
    
    if (strcmp(argv[1], "find") == 0) {
        const char *directory = NULL;
        const char *extension = NULL;
        const char *substring = NULL;
        
        int i = 2;
        while (i < argc) {
            if (strcmp(argv[i], "--dir") == 0 && i + 1 < argc) {
                directory = argv[i + 1];
                i = i + 2;
            }
            else if (strcmp(argv[i], "--ext") == 0 && i + 1 < argc) {
                extension = argv[i + 1];
                i = i + 2;
            }
            else if (strcmp(argv[i], "--name-contains") == 0 && i + 1 < argc) {
                substring = argv[i + 1];
                i = i + 2;
            }
            else {
                i++;
            }
        }
        
        return doFind(directory, extension, substring);
    }
    
    printf("ERROR: unknown command '%s'\n", argv[1]);
    showHelp();
    return 1;
}

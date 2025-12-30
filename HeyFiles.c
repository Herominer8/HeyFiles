#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
    #define CHANGE_DIR _chdir
#else
    #include <unistd.h>
    #define MKDIR(path) mkdir(path, 0755)
    #define CHANGE_DIR chdir
#endif

/* ================= Utility Functions ================= */

/* Safely read input from stdin */
static void read_input(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

/* ================= File Operations ================= */

/* List directory contents */
void list_files(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

/* Remove file or empty directory */
void remove_entry(const char *path) {
    struct stat info;

    if (stat(path, &info) != 0) {
        perror("stat failed");
        return;
    }

    if (S_ISDIR(info.st_mode)) {
        if (rmdir(path) == 0)
            printf("Directory '%s' removed successfully\n", path);
        else
            perror("rmdir failed");
    } else {
        if (remove(path) == 0)
            printf("File '%s' removed successfully\n", path);
        else
            perror("remove failed");
    }
}

/* Rename file inside a directory */
void rename_entry(const char *dir_path, const char *old_name, const char *new_name) {
    char old_path[PATH_MAX];
    char new_path[PATH_MAX];

    snprintf(old_path, sizeof(old_path), "%s/%s", dir_path, old_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", dir_path, new_name);

    if (rename(old_path, new_path) == 0)
        printf("Renamed successfully\n");
    else
        perror("rename failed");
}

/* Create directory */
void create_directory(const char *name) {
    if (MKDIR(name) == 0)
        printf("Directory created\n");
    else
        perror("mkdir failed");
}

/* Create empty file */
void create_file(const char *name) {
    FILE *file = fopen(name, "w");
    if (!file) {
        perror("fopen failed");
        return;
    }
    fclose(file);
    printf("File created\n");
}

/* Check existence */
void check_exists(const char *name) {
    struct stat info;
    if (stat(name, &info) == 0)
        printf("'%s' exists\n", name);
    else
        printf("'%s' does not exist\n", name);
}

/* Change current working directory */
void change_directory(const char *path) {
    if (CHANGE_DIR(path) == 0)
        printf("Current directory changed\n");
    else
        perror("chdir failed");
}

/* ================= Recursive Size Calculation ================= */

/* Recursively calculate directory size */
long long calculate_directory_size(const char *path) {
    DIR *dir = opendir(path);
    if (!dir)
        return 0;

    struct dirent *entry;
    struct stat info;
    char full_path[PATH_MAX];
    long long total_size = 0;

    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &info) != 0)
            continue;

        if (S_ISDIR(info.st_mode)) {
            /* Recurse into subdirectory */
            total_size += calculate_directory_size(full_path);
        } else {
            /* Add file size */
            total_size += info.st_size;
        }
    }

    closedir(dir);
    return total_size;
}

/* Get size of file or directory */
void get_size(const char *path) {
    struct stat info;

    if (stat(path, &info) != 0) {
        perror("stat failed");
        return;
    }

    if (S_ISDIR(info.st_mode)) {
        long long size = calculate_directory_size(path);
        printf("Directory size: %lld bytes\n", size);
    } else {
        printf("File size: %lld bytes\n", (long long)info.st_size);
    }
}

/* ================= Main Program ================= */

int main(void) {
    while (true) {
        printf("\n====== HeyFiles FILE MANAGER ======\n");
        printf("[0] List directory\n");
        printf("[1] Remove file/directory\n");
        printf("[2] Rename file\n");
        printf("[3] Create file/directory\n");
        printf("[4] Check existence\n");
        printf("[5] Change directory\n");
        printf("[6] Show size\n");
        printf("[9] Exit\n");
        printf("> ");

        char input[16];
        read_input(input, sizeof(input));
        int choice = atoi(input);

        if (choice == 9)
            break;

        char path[PATH_MAX];
        char name1[PATH_MAX];
        char name2[PATH_MAX];

        switch (choice) {
            case 0:
                printf("Path: ");
                read_input(path, sizeof(path));
                list_files(path);
                break;

            case 1:
                printf("Path: ");
                read_input(path, sizeof(path));
                remove_entry(path);
                break;

            case 2:
                printf("Directory: ");
                read_input(path, sizeof(path));
                printf("Old name: ");
                read_input(name1, sizeof(name1));
                printf("New name: ");
                read_input(name2, sizeof(name2));
                rename_entry(path, name1, name2);
                break;

            case 3:
                printf("[1] Directory\n[2] File\n> ");
                read_input(input, sizeof(input));
                if (atoi(input) == 1) {
                    printf("Directory name: ");
                    read_input(name1, sizeof(name1));
                    create_directory(name1);
                } else {
                    printf("File name: ");
                    read_input(name1, sizeof(name1));
                    create_file(name1);
                }
                break;

            case 4:
                printf("Name: ");
                read_input(name1, sizeof(name1));
                check_exists(name1);
                break;

            case 5:
                printf("Path: ");
                read_input(path, sizeof(path));
                change_directory(path);
                break;

            case 6:
                printf("Path: ");
                read_input(path, sizeof(path));
                get_size(path);
                break;

            default:
                printf("Invalid option\n");
        }
    }

    return 0;
}

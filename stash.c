#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define BREAK_MODE "break"
#define RESTORE_MODE "restore"

int main(int argc, char *argv[]) {
    char* mode = argv[1];
    char* file_path = argv[2];

    bool is_break_mode = strcmp(mode, BREAK_MODE) == 0;
    bool is_restore_mode = strcmp(mode, RESTORE_MODE) == 0;

    if (!is_break_mode && !is_restore_mode) {
        printf("You should pass %s or %s mode as a first agrument\n", BREAK_MODE, RESTORE_MODE);
        exit(1);
    }

    if (!file_path) {
        puts("You should pass file path as a second agrument");
        exit(1);
    }

    FILE* file;
    const unsigned char breaking_byte = 0x66;
    const int breaking_size = sizeof(breaking_byte);
    unsigned char buff[breaking_size];
    const int members_count = 1;

    file = fopen(file_path, "r+b");

    if (!file) {
        perror("You should pass correct file path as a first agrument");
        exit(1);
    }

    if (is_break_mode) {
        size_t read_members = fread(&buff, breaking_size, members_count, file);

        if (read_members == members_count) {
            if (breaking_byte == buff[0]) {
                puts("File is already corrupted");
            } else {
                if (fseek(file, -breaking_size, SEEK_CUR) != 0) {
                    perror("Error corrupting file");
                    exit(1);
                }
                if (fwrite(&breaking_byte, breaking_size, members_count, file) != members_count) {
                    perror("Error corrupting file");
                    exit(1);
                }
                if (fseek(file, 0, SEEK_END) != 0) {
                    perror("Error corrupting file");
                    exit(1);
                }
                if (fwrite(&buff, breaking_size, members_count, file) != members_count) {
                    perror("Error corrupting file");
                    exit(1);
                }
                puts("File was corrupted");
            }
        } else if(feof(file)) {
            puts("End of file reached");
            exit(1);
        } else {
            puts("Error corrupting file");
            exit(1);
        }
    }

    if (is_restore_mode) {
        unsigned char end_buff[breaking_size];

        size_t read_members = fread(&buff, breaking_size, members_count, file);

        if (read_members == members_count) {
            if (breaking_byte != buff[0]) {
                printf("File is not corrupted\n");
            } else {
                if (fseek(file, -breaking_size, SEEK_END) != 0) {
                    perror("Error restoring file");
                    exit(1);
                }
                if (fread(&end_buff, breaking_size, members_count, file) != members_count) {
                    perror("Error corrupting file");
                    exit(1);
                }
                if (fseek(file, -breaking_size, SEEK_END) != 0) {
                    perror("Error restoring file");
                    exit(1);
                }
                if (ftruncate(fileno(file), ftello(file)) != 0) {
                    perror("Error restoring file");
                    exit(1);
                }
                if (fseek(file, 0, SEEK_SET) != 0) {
                    perror("Error restoring file");
                    exit(1);
                }
                if (fwrite(&end_buff, breaking_size, members_count, file) != members_count) {
                    perror("Error corrupting file");
                    exit(1);
                }
                printf("File was restored\n");

            }
        } else if(feof(file)) {
            puts("End of file reached");
            exit(1);
        } else {
            puts("Error restoring file");
            exit(1);
        }
    }

    fclose(file);

    return 0;
}

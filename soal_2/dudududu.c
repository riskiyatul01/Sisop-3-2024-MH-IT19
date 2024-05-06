#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>


int stringkeint(char *str) {
    if (strcmp(str, "satu") == 0) return 1;
    else if (strcmp(str, "dua") == 0) return 2;
    else if (strcmp(str, "tiga") == 0) return 3;
    else if (strcmp(str, "empat") == 0) return 4;
    else if (strcmp(str, "lima") == 0) return 5;
    else if (strcmp(str, "enam") == 0) return 6;
    else if (strcmp(str, "tujuh") == 0) return 7;
    else if (strcmp(str, "delapan") == 0) return 8;
    else if (strcmp(str, "sembilan") == 0) return 9;
    else return -1;
}


void ubahkata(int num, char *kata) { //untuk mengubah angka menjadi huruf
    char *ones[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    char *tens[] = {"", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};

    if (num >= 1 && num <= 9) {
        strcpy(kata, ones[num]);
    } else if (num >= 10 && num <= 19) {
	strcpy(kata, teens[num - 10]);
    } else if (num >= 20 && num <= 99) {
        int ten = num / 10;
        int one = num % 10;
        if (one == 0) {
            strcpy(kata, tens[ten]);
        } else {
            sprintf(kata, "%s %s", tens[ten], ones[one]);
        }
    }
}

int main(int argc, char *argv[]) {
int num1,num2;
    if (argc != 2 || (strcmp(argv[1], "-kali") != 0 && strcmp(argv[1], "-tambah") != 0 && strcmp(argv[1], "-kurang") != 0 && strcmp(argv[1], "-bagi") != 0)) {
        printf("Usage: %s <-kali/-tambah/-kurang/-bagi>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int pipe_parent_to_child[2];
    int pipe_child_to_parent[2];

    if (pipe(pipe_parent_to_child) == -1 || pipe(pipe_child_to_parent) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        close(pipe_parent_to_child[0]);
        close(pipe_child_to_parent[1]);

        char input[20];
        printf("Masukkan dua angka (dalam kata): ");
        fgets(input, sizeof(input), stdin);
        strtok(input, "\n");

        char *token = strtok(input, " ");
        int num1 = stringkeint(token);
	token = strtok(NULL, " ");
        int num2 = stringkeint(token);

        int result;
        if (strcmp(argv[1], "-kali") == 0) {
            result = num1 * num2;
        } else if (strcmp(argv[1], "-tambah") == 0) {
            result = num1 + num2;
        } else if (strcmp(argv[1], "-kurang") == 0) {
            result = num1 - num2;
            if (result < 0) {
                printf("ERROR pada pengurangan.\n");
                exit(EXIT_FAILURE);
            }
        } else {
            if (num2 == 0) {
                printf("ERROR: Pembagian dengan nol\n");
                exit(EXIT_FAILURE);
            }
            result = num1 / num2;
            if (result < 0) {
                printf("ERROR pada pembagian.\n");
                exit(EXIT_FAILURE);
            }
        }

        write(pipe_parent_to_child[1], &result, sizeof(result));

        char message[200];
        read(pipe_child_to_parent[0], message, sizeof(message));
        printf("%s\n", message);

        close(pipe_parent_to_child[1]);
        close(pipe_child_to_parent[0]);

        // Log the result
        FILE *filelog = fopen("histori.log", "a");
        if (filelog != NULL) {
            char pesanlog[100];
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            strftime(pesanlog, sizeof(pesanlog), "[%d/%m/%y %H:%M:%S]", tm_info);
            fprintf(filelog, "%s [%s] %s\n", pesanlog, argv[1] + 1, message);
            fclose(filelog);
        } else {
            perror("fopen");
        }

        wait(NULL);
    } else { // proses untukchild
        close(pipe_parent_to_child[1]);
        close(pipe_child_to_parent[0]);

        int result;
        read(pipe_parent_to_child[0], &result, sizeof(result));

        char kata[100];
        ubahkata(result, kata);

        char message[150];
        if (strcmp(argv[1], "-kali") == 0) {
            sprintf(message, "hasil perkalian %d dan %d adalah %s.", num1, num2, kata);
        } else if (strcmp(argv[1], "-tambah") == 0) {
            sprintf(message, "hasil penjumlahan %d dan %d adalah %s.", num1, num2, kata);
        } else if (strcmp(argv[1], "-kurang") == 0) {
            sprintf(message, "hasil pengurangan %d dan %d adalah %s.", num1, num2, kata);
        } else {
            sprintf(message, "hasil pembagian %d dan %d adalah %s.", num1, num2, kata);
        }

        write(pipe_child_to_parent[1], message, sizeof(message));
        close(pipe_parent_to_child[0]);
        close(pipe_child_to_parent[1]);

        exit(EXIT_SUCCESS);
    }

	return 0;
}

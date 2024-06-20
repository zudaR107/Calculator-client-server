#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Проверка на корректное число
int is_valid_number(char *str) {
    int i = 0;
    // Обработчик отрицательных чисел 
    if (str[0] == '-') i = 1;
    int dot_count = 0;
    for (; str[i] != '\0'; i++) {
        if (str[i] == '.') {
            dot_count++;
            if (dot_count > 1) return 0;
            continue;
        }
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

// Проверка ввода 
int validate_input(char *input, double *operand1, char *operator, double *operand2) {
    char op1[50], op2[50];
    if (sscanf(input, "%s %c %s", op1, operator, op2) != 3) {
        return 0;
    }

    if (!is_valid_number(op1) || !is_valid_number(op2)) {
        return 0;
    }

    *operand1 = atof(op1);
    *operand2 = atof(op2);
    return 1;
}

void perform_operations(FILE *file, int sockfd) {
    char operator;
    double operand1, operand2, result;
    char input[100];

    // Лог времени подключения
    time_t now = time(NULL);
    char *start_time = ctime(&now);

    // Удаление \n из ctime
    start_time[strcspn(start_time, "\n")] = 0;  

    fprintf(file, "%s\n", start_time);
    fflush(file);

    while (1) {
        printf("Enter an expression (operand1 operator operand2) or 'exit' to finish\n");
        printf("Expression: ");
        fgets(input, sizeof(input), stdin);

        if (strncmp(input, "exit", 4) == 0) {
            break;
        }

        if (!validate_input(input, &operand1, &operator, &operand2)) {
            printf("Invalid input, please follow the format: operand1 operator operand2\n");
            continue;
        }

        switch(operator) {
            case '+':
                result = operand1 + operand2;
                break;
            case '-':
                result = operand1 - operand2;
                break;
            case '*':
                result = operand1 * operand2;
                break;
            case '/':
                if (operand2 == 0) {
                    printf("Division by zero is not allowed.\n");
                    continue;
                }
                result = operand1 / operand2;
                break;
            default:
                printf("Invalid operator! Use one of +, -, *, /\n");
                continue;
        }

        fprintf(file, "%lf %c %lf = %lf\n", operand1, operator, operand2, result);
        fflush(file);

        printf("Result: %lf\n", result);

        // Отправка результата на сервер
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%lf %c %lf = %lf\n", operand1, operator, operand2, result);
        int n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
            error("ERROR writing to socket");
    }

    fprintf(file, "\n");
    fflush(file);
}

int main(int argc, char *argv[]) {
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
        fprintf(stderr,"Usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // Перезапись логов одной сессии на стороне клиента 
    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen("results.txt", "w");
    if (file == NULL) 
        error("ERROR opening file");
    perform_operations(file, sockfd);
    pthread_mutex_unlock(&file_mutex);
    fclose(file);
    close(sockfd);
    return 0;
}

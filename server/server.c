#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>  // Include for inet_ntop

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void log_connection(struct sockaddr_in addr, FILE *file) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    time_t now = time(NULL);
    char *dt = ctime(&now);

    // Удаление \n из ctime
    dt[strcspn(dt, "\n")] = 0;  

    // Лог подключения
    fprintf(file, "%s at %s\n", client_ip, dt);  
    fflush(file);
    printf("Session started from %s at %s\n", client_ip, dt);
}

void log_disconnection(struct sockaddr_in addr, FILE *file) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    time_t now = time(NULL);
    char *dt = ctime(&now);

    // Удаление \n из ctime
    dt[strcspn(dt, "\n")] = 0;  

    // Добавление пустой строки в конце файла
    fprintf(file, "\n");  
    fflush(file);
    printf("Session ended from %s at %s\n", client_ip, dt);
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;  
    int read_size;
    char buffer[256];
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    getpeername(sock, (struct sockaddr*)&cli_addr, &cli_len);

    // Уникальное имя файла для одной сессии
    char session_filename[256];
    snprintf(session_filename, sizeof(session_filename), "session_%ld.txt", time(NULL));

    FILE *session_file = fopen(session_filename, "w");
    if (session_file == NULL) 
        error("ERROR opening session file");

    // Лог подключения клиента в файл сессии
    log_connection(cli_addr, session_file);

    // Получение данных от клиента и сохранение в файл сессии
    while ((read_size = read(sock, buffer, 255)) > 0) {  
        fwrite(buffer, 1, read_size, session_file);
    }

    log_disconnection(cli_addr, session_file);
    fclose(session_file);

    if (read_size == 0) {
        puts("Client disconnected");
    } else if (read_size == -1) {
        perror("recv failed");
    }

    // Добавление файла сессии в главный файл логов
    pthread_mutex_lock(&file_mutex);
    FILE *main_file = fopen("received_results.txt", "a");
    if (main_file == NULL) 
        error("ERROR opening main file");

    FILE *temp_file = fopen(session_filename, "r");
    if (temp_file == NULL) 
        error("ERROR opening temp session file");

    while ((read_size = fread(buffer, 1, sizeof(buffer), temp_file)) > 0) {
        fwrite(buffer, 1, read_size, main_file);
    }

    fclose(temp_file);
    fclose(main_file);
    pthread_mutex_unlock(&file_mutex);

    // Удаление временного файла сессии
    remove(session_filename);  

    close(sock);
    free(socket_desc);
    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, *new_sock;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t thread_id;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // Очистка файла 
    FILE *main_file = fopen("received_results.txt", "w");
    if (main_file == NULL) 
        error("ERROR opening main file");
    fclose(main_file);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");

        new_sock = malloc(sizeof(int));
        if (new_sock == NULL) 
            error("ERROR allocating memory for new socket");
        
        *new_sock = newsockfd;
        if (pthread_create(&thread_id, NULL, client_handler, (void*) new_sock) < 0) {
            error("could not create thread");
        }
        pthread_detach(thread_id);
    }

    close(sockfd);
    return 0;
}

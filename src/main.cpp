#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#define LOG(x) std::cout << x << std::endl
int main(int argc, char* argv[]) {
    // Disable output buffering
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket: " << std::endl;
        return 1;
    }

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        close(server_fd);
        std::cerr << "setsockopt failed: " << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9092);

    if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
        close(server_fd);
        std::cerr << "Failed to bind to port 9092" << std::endl;
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        close(server_fd);
        std::cerr << "listen failed" << std::endl;
        return 1;
    }

    std::cout << "Waiting for a client to connect...\n";

    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here!\n";
    
    // Uncomment this block to pass the first stage
    // 
    bool print_to_console =0;
    
    struct reqmodel {
        int32_t message_size;
        int16_t request_api_key;
        int16_t request_api_version;
        int32_t correlation_id;
    };
    struct resmodel {
        int32_t message_size;
        int32_t correlation_id;
        int16_t error_code;
    };
    struct reqmodel request;
    struct resmodel response;
    
    int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
    std::cout << "Client connected\n";
    std::byte rec_buffer[1024];
    //recv data
    recv(client_fd,&rec_buffer, 1024, 0);
    std::memcpy(&(request.message_size),rec_buffer,4);
    std::memcpy(&(request.request_api_key),rec_buffer+4,2);
    std::memcpy(&(request.request_api_version),rec_buffer+6,2);
    std::memcpy(&(request.correlation_id),rec_buffer+8,4);
    
    //convert bytes from network byte order(always big endian) to host order(intel, small endian)
    request.message_size=ntohl(request.message_size);
    request.request_api_key=ntohs(request.request_api_key);
    request.request_api_version=ntohs(request.request_api_version);
    request.correlation_id=ntohl(request.correlation_id);

    //Print to console
    if(print_to_console ==1){
        LOG(request.message_size);
        LOG(request.request_api_key);
        LOG(request.request_api_version);
        LOG(request.correlation_id);
    }


 
    //Logic for response calculation
    response.correlation_id=request.correlation_id;
    if(request.request_api_version != 4){
        response.error_code=35;
    }
    
    response.message_size=sizeof(response.correlation_id)+sizeof(response.error_code);


    //convert bytes from host order(intel, small endian) to network byte order(always big endian)
    response.correlation_id=htonl(response.correlation_id);
    response.error_code=htons(response.error_code);
    response.message_size=htonl(response.message_size);

    //send response
    send(client_fd,&(response.message_size),sizeof(response.message_size),0);
    send(client_fd,&(response.correlation_id),sizeof(response.correlation_id),0);
    send(client_fd,&(response.error_code),sizeof(response.error_code),0);
      
    close(client_fd);

    close(server_fd);
    return 0;
}
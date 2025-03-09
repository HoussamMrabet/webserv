/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:12:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/09 16:17:38 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#define PORT 3000
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE + 1] = {0};
    std::string buf;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("Socket failed");
        return EXIT_FAILURE;
    }

    // Forcefully bind to the port even if already in use
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        return EXIT_FAILURE;
    }

    // Configure address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // setsockopt(); // bad adress port
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        return EXIT_FAILURE;
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        return EXIT_FAILURE;
    }

    // std::cout << "Server listening on port " << PORT << "..." << std::endl;

    Request req;
    while (true)
    {
        // Accept connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        fcntl(new_socket, F_SETFL, O_NONBLOCK);
        // Read request
        while (req.getCurrentStep() != DONE)
        {
            ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread > 0)
            {
                buffer[valread] = '\0';
                // std::cout << "Received Request:\n"
                //           << std::endl;
                
                buf.append(buffer, valread);
                // std::cout.write(buf.c_str(), buf.size());
                // std::cout.flush();
                // std::cout << buf << std::flush;
                req.parseRequest(buf);
                // req.printRequest();
                buf.clear();
            }
            else
            {
                req.parseRequest();
            }
        }
        // req.printRequest();
        // Simple HTTP Response
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nDone!";
        send(new_socket, response.c_str(), response.length(), 0);

        // Close connection
        close(new_socket);
    }

    return 0;
}
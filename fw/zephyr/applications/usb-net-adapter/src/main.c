#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/net/net_config.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/zperf.h>

#define SEND_BUFFER_SIZE 1024
#define RECV_BUFFER_SIZE 1024

/* Network configuration parameters */
#define UDP_SERVER_PORT 5001
#define TCP_SERVER_PORT 5002
#define SERVER_ADDR "192.0.2.2"  // Replace with your target server address

/* Thread stack size and priority */
#define NETWORK_THREAD_STACK_SIZE 2048
#define NETWORK_THREAD_PRIORITY 5

/* UDP send thread */
void udp_send_thread(void *arg1, void *arg2, void *arg3) {
    int sock;
    struct sockaddr_in server_addr;
    char send_buffer[SEND_BUFFER_SIZE];

    /* Prepare server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);

    /* Create UDP socket */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printk("UDP: Failed to create socket: %d\n", errno);
        return;
    }

    sprintf(send_buffer, "%s", "UDP Packet:");

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Fill the buffer with random printable ASCII characters
    for (size_t i = 0; i < SEND_BUFFER_SIZE-11; i++) {
        send_buffer[i+11] = (char)(32 + rand() % 95); // Printable ASCII range is 32 to 126
    }

    send_buffer[SEND_BUFFER_SIZE - 1] = '\0'; // Null-terminate the buffer
    
    while (1) {
        int sent_bytes = sendto(sock, send_buffer, SEND_BUFFER_SIZE, 0, 
                                (struct sockaddr *)&server_addr, 
                                sizeof(server_addr));
        
        if (sent_bytes < 0) {
            printk("UDP Send failed: %d\n", errno);
            k_msleep(5000);  
            continue;
        }

        printk("UDP Sent %d bytes\n", sent_bytes);
        k_msleep(5000);
    }

    close(sock);
}

/* UDP receive thread */
void udp_recv_thread(void *arg1, void *arg2, void *arg3) {
    int sock;
    struct sockaddr_in bind_addr;
    char recv_buffer[RECV_BUFFER_SIZE];

    /* Prepare bind address */
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(UDP_SERVER_PORT);
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    /* Create UDP socket */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printk("UDP: Failed to create receive socket: %d\n", errno);
        return;
    }

    /* Bind socket */
    if (bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        printk("UDP Bind failed: %d\n", errno);
        close(sock);
        return;
    }

    while (1) {
        int received_bytes = recv(sock, recv_buffer, RECV_BUFFER_SIZE, 0);
        
        if (received_bytes < 0) {
            printk("UDP Receive failed: %d\n", errno);
            k_msleep(5000);  
            continue;
        }

        printk("UDP Received %d bytes\n", received_bytes);
    }

    close(sock);
}

/* TCP send thread */
void tcp_send_thread(void *arg1, void *arg2, void *arg3) {
    int sock;
    struct sockaddr_in server_addr;
    char send_buffer[SEND_BUFFER_SIZE];
    bool connected = false;

    /* Prepare server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);

    while (1) {
        if (!connected) {
            /* Create TCP socket */
            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock < 0) {
                printk("TCP: Failed to create socket: %d\n", errno);
                k_msleep(5000);
                continue;
            }

            /* Connect to server */
            if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                printk("TCP: Connection failed: %d\n", errno);
                close(sock);
                k_msleep(5000);
                continue;
            }
            connected = true;
            printk("TCP: Connected to server\n");
        }

        sprintf(send_buffer, "%s", "TCP Packet:");

        // Seed the random number generator
        srand((unsigned int)time(NULL));

        // Fill the buffer with random printable ASCII characters
        for (size_t i = 0; i < SEND_BUFFER_SIZE - 11; i++) {
            send_buffer[i + 11] = (char)(32 + rand() % 95); // Printable ASCII range is 32 to 126
        }

        send_buffer[SEND_BUFFER_SIZE - 1] = '\0'; // Null-terminate the buffer

        /* Send data */
        int sent_bytes = send(sock, send_buffer, SEND_BUFFER_SIZE, 0);
        if (sent_bytes < 0) {
            printk("TCP Send failed: %d, reconnecting...\n", errno);
            close(sock);
            connected = false;
            k_msleep(5000);  // Delay before reconnecting
            continue;
        }

        printk("TCP Sent %d bytes\n", sent_bytes);
        k_msleep(5000);  // Delay between sends
    }
}

/* TCP receive thread */
void tcp_recv_thread(void *arg1, void *arg2, void *arg3) {
    int server_sock, client_sock;
    struct sockaddr_in bind_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char recv_buffer[RECV_BUFFER_SIZE];

    /* Prepare bind address */
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(TCP_SERVER_PORT);
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket */
    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock < 0) {
        printk("TCP: Failed to create server socket: %d\n", errno);
        return;
    }

    /* Bind socket */
    if (bind(server_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        printk("TCP Bind failed: %d\n", errno);
        close(server_sock);
        return;
    }

    /* Listen for connections */
    if (listen(server_sock, 1) < 0) {
        printk("TCP Listen failed: %d\n", errno);
        close(server_sock);
        return;
    }

    while (1) {
        /* Accept incoming connection */
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            printk("TCP Accept failed: %d\n", errno);
            k_msleep(5000);
            continue;
        }

        int received_bytes = recv(client_sock, recv_buffer, RECV_BUFFER_SIZE, 0);
        
        if (received_bytes < 0) {
            printk("TCP Receive failed: %d\n", errno);
        } else {
            printk("TCP Received %d bytes\n", received_bytes);
        }

        close(client_sock);
    }

    close(server_sock);
}


/* UDP Echo Thread */
void udp_echo_thread(void *arg1, void *arg2, void *arg3) {
    int sock;
    struct sockaddr_in bind_addr, client_addr;
    char recv_buffer[RECV_BUFFER_SIZE];
    socklen_t client_addr_len = sizeof(client_addr);

    /* Prepare bind address */
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(UDP_SERVER_PORT);
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    /* Create UDP socket */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printk("UDP: Failed to create socket: %d\n", errno);
        return;
    }

    /* Bind socket */
    if (bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        printk("UDP Bind failed: %d\n", errno);
        close(sock);
        return;
    }

    while (1) {
        /* Receive data */
        int received_bytes = recvfrom(sock, recv_buffer, RECV_BUFFER_SIZE, 0,
                                      (struct sockaddr *)&client_addr, &client_addr_len);
        if (received_bytes < 0) {
            printk("UDP Receive failed: %d\n", errno);
            k_msleep(5000);
            continue;
        }

        printk("UDP Received %d bytes: %.*s\n", received_bytes, received_bytes, recv_buffer);

        /* Echo back the received data */
        int sent_bytes = sendto(sock, recv_buffer, received_bytes, 0,
                                (struct sockaddr *)&client_addr, client_addr_len);
        if (sent_bytes < 0) {
            printk("UDP Echo failed: %d\n", errno);
        } else {
            printk("UDP Echoed %d bytes\n", sent_bytes);
        }
    }

    close(sock);
}

/* TCP Echo Thread */
void tcp_echo_thread(void *arg1, void *arg2, void *arg3) {
    int server_sock, client_sock;
    struct sockaddr_in bind_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char recv_buffer[RECV_BUFFER_SIZE];

    /* Prepare bind address */
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(TCP_SERVER_PORT);
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket */
    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock < 0) {
        printk("TCP: Failed to create server socket: %d\n", errno);
        return;
    }

    /* Bind socket */
    if (bind(server_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        printk("TCP Bind failed: %d\n", errno);
        close(server_sock);
        return;
    }

    /* Listen for connections */
    if (listen(server_sock, 1) < 0) {
        printk("TCP Listen failed: %d\n", errno);
        close(server_sock);
        return;
    }

    while (1) {
        /* Accept incoming connection */
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            printk("TCP Accept failed: %d\n", errno);
            k_msleep(5000);
            continue;
        }

        printk("TCP: Client connected\n");

        /* Echo data back to the client */
        while (1) {
            int received_bytes = recv(client_sock, recv_buffer, RECV_BUFFER_SIZE, 0);
            if (received_bytes < 0) {
                printk("TCP Receive failed: %d\n", errno);
                break;
            }
            if (received_bytes == 0) {
                printk("TCP: Client disconnected\n");
                break;
            }

            printk("TCP Received %d bytes: %.*s\n", received_bytes, received_bytes, recv_buffer);

            int sent_bytes = send(client_sock, recv_buffer, received_bytes, 0);
            if (sent_bytes < 0) {
                printk("TCP Echo failed: %d\n", errno);
                break;
            }

            printk("TCP Echoed %d bytes\n", sent_bytes);
        }

        close(client_sock);
    }

    close(server_sock);
}


/* UDP Receive Thread  */
// K_THREAD_DEFINE(udp_send_thread_id, NETWORK_THREAD_STACK_SIZE,
//                 udp_send_thread, NULL, NULL, NULL,
//                 NETWORK_THREAD_PRIORITY, 0, 0);

/* UDP Receive Thread  */
// K_THREAD_DEFINE(udp_recv_thread_id, NETWORK_THREAD_STACK_SIZE,
//                 udp_recv_thread, NULL, NULL, NULL,
//                 NETWORK_THREAD_PRIORITY, 0, 0);

/* TCP Send Thread  */
// K_THREAD_DEFINE(tcp_send_thread_id, NETWORK_THREAD_STACK_SIZE,
//                 tcp_send_thread, NULL, NULL, NULL,
//                 NETWORK_THREAD_PRIORITY, 0, 0);

/* TCP Receive Thread  */
// K_THREAD_DEFINE(tcp_recv_thread_id, NETWORK_THREAD_STACK_SIZE,
//                 tcp_recv_thread, NULL, NULL, NULL,
//                 NETWORK_THREAD_PRIORITY, 0, 0);


/* UDP Echo Thread  */
K_THREAD_DEFINE(udp_echo_thread_id, NETWORK_THREAD_STACK_SIZE,
                udp_echo_thread, NULL, NULL, NULL,
                NETWORK_THREAD_PRIORITY, 0, 0);


/* TCP Echo Thread  */
K_THREAD_DEFINE(tcp_echo_thread_id, NETWORK_THREAD_STACK_SIZE,
                tcp_echo_thread, NULL, NULL, NULL,
                NETWORK_THREAD_PRIORITY, 0, 0);

int main(void) {
    printf("USB-Net dongle\n");
    int ret = usb_enable(NULL);    
    if (ret != 0) {
        printk("USB enable error %d\n", ret);
        return ret;
    }

    /* Network Initialization */
    ret = net_config_init_app(NULL, "Initializing network");
    if (ret < 0) {
        printk("Network initialization failed: %d\n", ret);
        return ret;
    }

    return 0;
}
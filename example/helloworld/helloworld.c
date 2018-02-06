/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include <string.h>

#include <aos/aos.h>
#include <aos/network.h>
#include <netmgr.h>

#define BUFFER_MAX_SIZE  1512
#define TCP_DEMO_TARGET_TCP_PORT 443

static void handle_lwiptestcmd(char *pwbuf, int blen, int argc, char **argv)
{
    char *ptype = argc > 1 ? argv[1] : "default";
    char buf[BUFFER_MAX_SIZE] = {0};
    int  readlen = 0;
    int  g_fd;
    char *pip, *pport, *pdata;
    ssize_t siz;
    int time = 0;
    struct sockaddr_in addr;
    
    /* TCP client case */
    if (strcmp(ptype, "tcp_c") == 0) {
        g_fd = socket(AF_INET,SOCK_STREAM,0);

        pip = argv[2];
        pport = argv[3];
        pdata = argv[4];

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((short)atoi(pport));
        addr.sin_addr.s_addr = inet_addr(pip);

        if (connect(g_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            LOGE(TAG, "Connect failed, errno = %d", errno);
            close(g_fd);
            return;
        }
        
        while(1){
            // send-recv
            if ((siz = send(g_fd, pdata, strlen(pdata), 0)) <= 0) {
                LOGE(TAG, "send failed, errno = %d.", errno);
                close(g_fd);
                return;
            }
            
            readlen = read(g_fd, buf, BUFFER_MAX_SIZE - 1);
            if (readlen < 0){
                LOGE(TAG, "recv failed, errno = %d.", errno);
                close(g_fd);
                return;
            }

            if (readlen == 0){
                LOGE(TAG, "recv buf len is %d \n", readlen);
                break;
            }
            
            LOGD(TAG, "recv server reply len %d info %s \n", readlen, buf);
            if (strstr(buf, pdata)){
                LOGI(TAG, "Goodbye! See you! (%d)\n", g_fd);
                time++;
            }
            
            if (time >= 100){
                break;
            }
        }
        
        close(g_fd);
        LOGI(TAG, "tcp_c test successful.");
    }else if (strcmp(ptype, "udp_c") == 0) {
        g_fd = socket(AF_INET,SOCK_DGRAM,0);

        pip = argv[2];
        pport = argv[3];
        pdata = argv[4];

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((short)atoi(pport));
        addr.sin_addr.s_addr = inet_addr(pip);

        siz = sendto(g_fd, pdata, strlen(pdata), 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
        if (siz <= 0){
            LOGE(TAG, "sendto failed, errno = %d.", errno);
            close(g_fd);
            return;
        }
        close(g_fd);
        LOGI(TAG, "udp_c test successful.");
    } else {
        char domain[] = "www.baidu.com";
        int port = 8080;
        struct hostent *host = NULL;
        struct timeval timeout;

        struct sockaddr_in server_addr;

        if ((host = gethostbyname(domain)) == NULL) {
            LOGE(TAG, "gethostbyname failed, errno: %d", errno);
            return;
        }
        printf("get target IP is %d.%d.%d.%d\n", (unsigned char)((*(unsigned long *)(host->h_addr) & 0x000000ff) >> 0),
                                            (unsigned char)((*(unsigned long *)(host->h_addr) & 0x0000ff00) >> 8),
                                            (unsigned char)((*(unsigned long *)(host->h_addr) & 0x00ff0000) >> 16),
                                            (unsigned char)((*(unsigned long *)(host->h_addr) & 0xff000000) >> 24));
        if ((g_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            LOGE(TAG, "Socket failed, errno: %d", errno);
            return;
        }

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        if (setsockopt (g_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                        sizeof(timeout)) < 0) {
            LOGE(TAG, "setsockopt failed, errno: %d", errno);
            close(g_fd);
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(TCP_DEMO_TARGET_TCP_PORT);
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        printf("connect to ip 0x%x port 0x%x \r\n", server_addr.sin_addr.s_addr, server_addr.sin_port);
        
        if (connect(g_fd, (struct sockaddr *) (&server_addr),
            sizeof(struct sockaddr)) == -1) {
            LOGE(TAG, "Connect failed, errno: %d", errno);
            close(g_fd);
            return;
        }
        close(g_fd);
        LOGI(TAG, "lwip domain test successful.");
    } 
}


static struct cli_command lwiptestcmds[] = {
    {
        .name = "lwip",
        .help = "lwip tcp_c|udp_c [remote_ip remote_port data]",
        .function = handle_lwiptestcmd
    }
};

static void wifi_event_handler(input_event_t *event, void *priv_data)
{
    if (event->type != EV_WIFI) return;
    if (event->code == CODE_WIFI_ON_PRE_GOT_IP)
        LOG("Hello, WiFi PRE_GOT_IP event!");
    if (event->code == CODE_WIFI_ON_GOT_IP)
        LOG("Hello, WiFi GOT_IP event!");
}

static void app_delayed_action(void *arg)
{
    LOG("%s:%d %s\r\n", __func__, __LINE__, aos_task_name());
    //aos_post_delayed_action(5000, app_delayed_action, NULL);
}

int application_start(int argc, char *argv[])
{
    netmgr_init();
    netmgr_start(false);
    aos_register_event_filter(EV_WIFI, wifi_event_handler, NULL);
    
    aos_cli_register_commands((const struct cli_command *)&lwiptestcmds[0],
      sizeof(lwiptestcmds) / sizeof(lwiptestcmds[0]));
    LOG("%s:%d %s\r\n", __func__, __LINE__, aos_task_name());
    //aos_post_delayed_action(1000, app_delayed_action, NULL);
    aos_loop_run();
    
    return 0;
}


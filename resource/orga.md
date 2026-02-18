1
gerer le parsing d'un fichier .conf

2
gestion des CGI

3
Gerer les socket

---


```mermaid

flowchart TB
    A["Config file"] --> B["Parser"]
    B --> CFG[("ServerConfig[]")]
    CFG --> EPOLL["Socket / Epoll"]
    EPOLL --> LOOP["Main loop"]
    LOOP --> WAIT["epoll_wait()"]
    WAIT -- server_fd ready --> ACCEPT["accept()"]
    ACCEPT --> NEWCLIENT["new client_fd<br>add to epoll"]
    NEWCLIENT --> WAIT
    WAIT -- client_fd ready --> READ["read(HTTP request)"]
    READ --> PARSE["parse request<br>method · path · headers"]
    PARSE -- look up route in ServerConfig --> CFG
    PARSE --> METHOD_SWITCH{"Method?"}
    METHOD_SWITCH -- GET --> GET_HANDLER["GET<br>static file or CGI"]
    METHOD_SWITCH -- POST --> POST_HANDLER["POST<br>upload or CGI"]
    METHOD_SWITCH -- DELETE --> DELETE_HANDLER["DELETE<br>unlink file"]
    GET_HANDLER --> CHECK_CGI{"CGI?"}
    CHECK_CGI -- yes --> CGIHANDLER["CGI Handler"]
    CHECK_CGI -- no --> RESP["HTTP Response"]
    POST_HANDLER --> CHECK_CGI
    DELETE_HANDLER --> RESP
    CGIHANDLER --> FORK["fork() + execve()<br>pipes + env"]
    FORK --> OUTPUT["script output"]
    OUTPUT --> RESP
    RESP --> WRITE["write(client_fd)"]
    WRITE --> CLOSE["close(client_fd)"]
    CLOSE --> WAIT

    EPOLL@{ shape: rounded}
    CGIHANDLER@{ shape: rounded}
    RESP@{ shape: rounded}
```
 
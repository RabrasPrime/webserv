> *This project has been created as part of the 42 curriculum by samaouch, tjooris, ale-guel.*
<!-- Custom fonts -->
<!-- 𝔸 𝔹 ℂ 𝔻 𝔼 𝔽 𝔾 ℍ 𝕀 𝕁 𝕂 𝕃 𝕄 ℕ 𝕆 ℙ ℚ ℝ 𝕊 𝕋 𝕌 𝕍 𝕎 𝕏 𝕐 ℤ -->

<div align="center">
  <b><font size="7">𝕎𝕖𝕓𝕤𝕖𝕣𝕧</font></b>
  <p><i>A C++98 Non-blocking HTTP/1.1 Server</i></p>
</div>

---


<details open>
    <summary>
        <h4>𝕊ummary</h4> 
    </summary>
<blockquote>

- [𝔻escription](#description)
    - [Core Features](#core)
    - [Technical Constraints](#constraints)
    - [Team Workflow](#team)
    - [System Architecture](#graph)
- [𝕀nstructions](#instructions)
    - [Compilation](#compilation)
    - [Execution](#usage)
    - [Configuration files](#config)
- [ℝesources](#resources)
- [𝔸dditional sections](#add)
</blockquote>
</details>

<b><font size="2">*end*[^1]</b></font>

--- 

<details id="description">
    <summary>
        <h2>𝔻escription</h2>
    </summary>

<blockquote>

This project involves developing an **HTTP/1.1, HTTP/1.0 Web Server** from scratch. The primary goal is to gain a deep understanding of the HTTP protocol, socket programming, and the underlying mechanics of the internet.

<dl><dd>
<details id="core">
    <summary>
        <h3>Core Features</h3>
    </summary>

<blockquote>

- **Multi-Port Listening:** Simultaneously manage multiple virtual servers on different ports.
- **HTTP Methods:** Full support for `GET`, `POST`, and `DELETE`.
- **Static Content:** Serve complete static websites with custom error page management.
- **File Upload:** Enable clients to upload files directly to the server.
- **CGI Engine:** Execute dynamic scripts (Python, PHP, etc.) via specific file extensions.
- **Directory Listing:** Automatically generate index pages for directories when enabled.

</blockquote>
</details>
</dd></dl>


<dl><dd>
<details id="constraints">
    <summary>
        <h3>Technical Constraints</h3>
    </summary>

<blockquote>

| Category | Specification |
|:-------|:------:| 
| **Standard** | C++ 98 |
| **I/O Model** | Non-blocking using `epoll` (Single instance) |
| **Protocol** | HTTP/1.1, HTTP/1.0 (Chunked encoding supported) |
| **Stability** | Zero crashes, zero leaks, no indefinite hangs |
| **Processes** | `fork()` used exclusively for CGI execution |

</blockquote>
</details>
</dd></dl>


<dl><dd>
<details id="team">
    <summary>
        <h3>Team Workflow</h3>
    </summary>

<blockquote>

We chose **epoll** for efficient socket event management. 
Our workflow started with individual research, followed by a collaborative phase where we identified four main modules: **Configuration Parsing**, **Socket/Epoll Management**, **Request Parsing**, and **Response Generation**.

- **Tools:** Git, Mermaid.live, Discord.
- **Communication:** Weekly in-person meetings, continuous sync via Discord.
- **Standards:** Clean commit history and mandatory Peer-Review (PR) before merging.

</blockquote>
</details>
</dd></dl>

<dl><dd>
<details id="graph">
    <summary>
        <h3>👀 System Architecture</h3>
    </summary>

<blockquote>

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
    WRITE --> CLOSE["Check Connection: <br> close or wait"]
    CLOSE --> WAIT

    EPOLL@{ shape: rounded}
    CGIHANDLER@{ shape: rounded}
    RESP@{ shape: rounded}
```

</blockquote>
</details>
</dd></dl>

</blockquote>
</details>

---

<details id="instructions">
    <summary>
        <h2>𝕀nstructions</h2>
    </summary>

<blockquote>

`` Section containing any relevant informaton about compilation, installation, and/or execution. ``

<dd><dl>
<details id="compilation">
    <summary>
        <h4>Compilation</h4>
    </summary>

<blockquote>

</blockquote>
</details>
</dd></dl>


<dd><dl>
<details id="usage">
    <summary>
        <h4>Execution</h4>
    </summary>

<blockquote>

</blockquote>
</details>
</dd></dl>

<dd><dl>
<details id="config">
    <summary>
        <h4>Configuration files</h4>
    </summary>

<blockquote>

</blockquote>
</details>
</dd></dl>

</blockquote>
</details>


---

<details id="ressources">
    <summary>
        <h2>ℝessources</h2>
    </summary>

`` Section listing classic references related to the topic (documentation, articles, tutorials, etc.), as well as description of how AI was used specifying for which tasks and which parts of the project. ``
</details>

---

<details id="add">
    <summary>
        <h2>𝔸dditional sections</h2>
    </summary>


`` (e.g, usage examples, feature list, technical choices, etc.). ``

</details>

[^1]:*begin*

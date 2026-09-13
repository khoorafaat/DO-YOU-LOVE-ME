/* --------------------------------------------------------------
 *  do_you_love_me.c
 *
 *  A tiny HTTP server (single-threaded, POSIX sockets) that
 *  serves a viral "Do You Love Me?" single-page web app.
 *
 *  Build:   gcc -Wall -O2 -o do_you_love_me do_you_love_me.c
 *  Run:     ./do_you_love_me          # listens on 127.0.0.1:8081
 *
 *  Open a browser at http://127.0.0.1:8081/
 *
 *  -------------------------------------------------------------- */

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* ----------------------------------------------------------------
 *  HTML page - everything (CSS & JS) is inlined.
 * ---------------------------------------------------------------- */
static const char *const page_html =
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"  <meta charset=\"UTF-8\"/>\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>\n"
"  <title>Do You Love Me?</title>\n"
"  <link href=\"https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap\" rel=\"stylesheet\"/>\n"
"  <style>\n"
"    *{margin:0;padding:0;box-sizing:border-box;}\n"
"    html,body{\n"
"      height:100%;\n"
"      font-family:'Poppins',sans-serif;\n"
"      overflow:hidden;\n"
"      background:linear-gradient(135deg,#ff9a9e,#fad0c4);\n"
"      background-size:400% 400%;\n"
"      animation:gradientShift 15s ease infinite;\n"
"      color:#222;\n"
"    }\n"
"    @keyframes gradientShift{\n"
"      0%{background-position:0% 50%;}\n"
"      50%{background-position:100% 50%;}\n"
"      100%{background-position:0% 50%;}\n"
"    }\n"
"    /* -- Card in center -- */\n"
"    .card-wrap{\n"
"      position:absolute;\n"
"      top:50%;left:50%;\n"
"      transform:translate(-50%,-50%);\n"
"      text-align:center;\n"
"      z-index:1;\n"
"    }\n"
"    .card{\n"
"      background:rgba(255,255,255,0.25);\n"
"      backdrop-filter:blur(12px);\n"
"      border-radius:24px;\n"
"      padding:2.5rem 2rem;\n"
"      box-shadow:0 8px 32px rgba(31,38,135,0.37);\n"
"      min-width:320px;\n"
"    }\n"
"    h1{font-size:2rem;margin-bottom:0.5rem;}\n"
"    .counter,.funny-message{margin:0.4rem 0;font-size:0.9rem;}\n"
"    .funny-message{color:#c0392b;font-weight:600;}\n"
"    .hidden{display:none;}\n"
"    /* -- Yes button stays in card -- */\n"
"    #yesBtn{\n"
"      margin-top:1rem;\n"
"      padding:0.75rem 2.5rem;\n"
"      font-size:1.1rem;font-weight:600;\n"
"      border:none;border-radius:14px;\n"
"      cursor:pointer;\n"
"      background:#ff6b81;color:#fff;\n"
"      box-shadow:0 4px 10px rgba(0,0,0,0.15);\n"
"      transition:transform 0.2s;\n"
"    }\n"
"    #yesBtn:hover{transform:scale(1.08);}\n"
"    /* -- No button is FIXED on the viewport -- */\n"
"    #noBtn{\n"
"      position:fixed;\n"
"      padding:0.75rem 2rem;\n"
"      font-size:1.1rem;font-weight:600;\n"
"      border:none;border-radius:14px;\n"
"      cursor:pointer;\n"
"      background:#ffb3b3;color:#fff;\n"
"      box-shadow:0 4px 10px rgba(0,0,0,0.15);\n"
"      z-index:10;\n"
"      transition:top 0.35s ease, left 0.35s ease;\n"
"      user-select:none;\n"
"    }\n"
"    /* -- Confetti & hearts -- */\n"
"    .confetti-canvas{position:fixed;top:0;left:0;width:100%;height:100%;pointer-events:none;z-index:999;}\n"
"    @keyframes floatHeart{\n"
"      0%{transform:translateY(0) scale(0.8);opacity:1;}\n"
"      100%{transform:translateY(-100vh) scale(1.4);opacity:0;}\n"
"    }\n"
"    @keyframes shake{\n"
"      0%{transform:translateX(0);}\n"
"      25%{transform:translateX(-6px);}\n"
"      50%{transform:translateX(6px);}\n"
"      75%{transform:translateX(-6px);}\n"
"      100%{transform:translateX(0);}\n"
"    }\n"
"    .shake{animation:shake 0.5s;}\n"
"    .footer{position:fixed;bottom:12px;left:0;width:100%;text-align:center;font-size:0.85rem;color:#555;font-weight:600;letter-spacing:0.5px;z-index:1;}\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"  <!-- Card -->\n"
"  <div class=\"card-wrap\">\n"
"    <div class=\"card\">\n"
"      <h1 id=\"question\">Do you love me? &#x1F97A;</h1>\n"
"      <p id=\"funny-message\" class=\"hidden\"></p>\n"
"      <div class=\"counter\">Attempts: <span id=\"attempts\">0</span></div>\n"
"      <button id=\"yesBtn\">Yes &#x1F496;</button>\n"
"    </div>\n"
"  </div>\n"
"  <!-- No button lives OUTSIDE the card, fixed on viewport -->\n"
"  <button id=\"noBtn\">No &#x1F622;</button>\n"
"  <div class=\"footer\">Made with &#x2764; love by ASHISH SINGHA MAHAPATRA</div>\n"
"  <canvas id=\"confettiCanvas\" class=\"confetti-canvas\"></canvas>\n"
"\n"
"  <script>\n"
"  (function(){\n"
"    var noBtn = document.getElementById('noBtn');\n"
"    var yesBtn = document.getElementById('yesBtn');\n"
"    var attemptsSpan = document.getElementById('attempts');\n"
"    var funnyMsg = document.getElementById('funny-message');\n"
"    var questionHeader = document.getElementById('question');\n"
"    var confettiCanvas = document.getElementById('confettiCanvas');\n"
"    var ctx = confettiCanvas.getContext('2d');\n"
"    var attempts = 0;\n"
"    var alive = true;\n"
"\n"
"    var funnyMessages = [\n"
"      'Are you sure? &#x1F914;',\n"
"      'Think again! &#x1F60F;',\n"
"      'You cannot escape love! &#x1F609;',\n"
"      'Nice try! &#x1F602;',\n"
"      'Still trying? Really? &#x1F644;',\n"
"      'The button is faster than you! &#x1F3C3;',\n"
"      'Give up already! &#x1F49E;',\n"
"      'Resistance is futile! &#x1F496;'\n"
"    ];\n"
"\n"
"    function randInt(a,b){return Math.floor(Math.random()*(b-a+1))+a;}\n"
"\n"
"    /* Place button at a random SAFE spot inside the viewport */\n"
"    function moveNoButton(){\n"
"      if(!alive) return;\n"
"      var bw = noBtn.offsetWidth  || 120;\n"
"      var bh = noBtn.offsetHeight || 48;\n"
"      var pad = 10;\n"
"      var maxX = window.innerWidth  - bw - pad;\n"
"      var maxY = window.innerHeight - bh - pad;\n"
"      if(maxX < pad) maxX = pad;\n"
"      if(maxY < pad) maxY = pad;\n"
"      var nx = randInt(pad, maxX);\n"
"      var ny = randInt(pad, maxY);\n"
"      noBtn.style.left = nx + 'px';\n"
"      noBtn.style.top  = ny + 'px';\n"
"    }\n"
"\n"
"    /* On hover or click: move + count attempt */\n"
"    function dodgeAndCount(){\n"
"      if(!alive) return;\n"
"      attempts++;\n"
"      attemptsSpan.textContent = attempts;\n"
"      moveNoButton();\n"
"      if(attempts >= 3){\n"
"        funnyMsg.classList.remove('hidden');\n"
"        funnyMsg.innerHTML = funnyMessages[randInt(0, funnyMessages.length-1)];\n"
"      }\n"
"    }\n"
"\n"
"    /* Initial position */\n"
"    noBtn.style.left = (window.innerWidth/2  - 60) + 'px';\n"
"    noBtn.style.top  = (window.innerHeight/2 + 60) + 'px';\n"
"\n"
"    /* Events: hover dodges, click dodges */\n"
"    noBtn.addEventListener('mouseenter', dodgeAndCount);\n"
"    noBtn.addEventListener('click', function(e){ e.preventDefault(); dodgeAndCount(); });\n"
"    /* Also handle touch for mobile */\n"
"    noBtn.addEventListener('touchstart', function(e){ e.preventDefault(); dodgeAndCount(); }, {passive:false});\n"
"\n"
"    /* Periodic gentle wander every 2s */\n"
"    setInterval(function(){ if(alive) moveNoButton(); }, 2000);\n"
"\n"
"    /* ---- Canvas resize ---- */\n"
"    function resizeCanvas(){confettiCanvas.width=window.innerWidth;confettiCanvas.height=window.innerHeight;}\n"
"    window.addEventListener('resize',resizeCanvas);\n"
"    resizeCanvas();\n"
"\n"
"    /* ---- Celebration ---- */\n"
"    function doShake(){document.body.classList.add('shake');setTimeout(function(){document.body.classList.remove('shake');},500);}\n"
"\n"
"    function Confetti(){\n"
"      this.x=Math.random()*confettiCanvas.width;\n"
"      this.y=-10;\n"
"      this.size=randInt(4,8);\n"
"      this.speedY=randInt(2,5);\n"
"      this.speedX=randInt(-2,2);\n"
"      this.rot=0;\n"
"      this.rotSp=randInt(-5,5);\n"
"      this.col='hsl('+randInt(0,360)+',70%,60%)';\n"
"    }\n"
"    Confetti.prototype.update=function(){this.x+=this.speedX;this.y+=this.speedY;this.rot+=this.rotSp;};\n"
"    Confetti.prototype.draw=function(){ctx.save();ctx.translate(this.x,this.y);ctx.rotate(this.rot*Math.PI/180);ctx.fillStyle=this.col;ctx.fillRect(-this.size/2,-this.size/2,this.size,this.size);ctx.restore();};\n"
"\n"
"    var confetti=[], animId=null;\n"
"    function startConfetti(duration){\n"
"      duration=duration||4000;\n"
"      var start=Date.now();\n"
"      function loop(){\n"
"        var elapsed=Date.now()-start;\n"
"        if(elapsed>duration){cancelAnimationFrame(animId);ctx.clearRect(0,0,confettiCanvas.width,confettiCanvas.height);confetti=[];return;}\n"
"        if(confetti.length<200) confetti.push(new Confetti());\n"
"        ctx.clearRect(0,0,confettiCanvas.width,confettiCanvas.height);\n"
"        for(var i=confetti.length-1;i>=0;i--){confetti[i].update();confetti[i].draw();if(confetti[i].y>confettiCanvas.height+10)confetti.splice(i,1);}\n"
"        animId=requestAnimationFrame(loop);\n"
"      }\n"
"      loop();\n"
"    }\n"
"\n"
"    function launchHearts(){\n"
"      for(var i=0;i<30;i++){\n"
"        var h=document.createElement('div');\n"
"        h.innerHTML='&#x1F496;';\n"
"        h.style.cssText='position:fixed;bottom:-30px;font-size:'+randInt(14,28)+'px;opacity:'+Math.random()+';pointer-events:none;left:'+randInt(0,window.innerWidth)+'px;animation:floatHeart '+randInt(2000,4000)+'ms linear forwards;';\n"
"        document.body.appendChild(h);\n"
"        setTimeout(function(el){return function(){el.remove();};}(h),5000);\n"
"      }\n"
"    }\n"
"\n"
"    yesBtn.addEventListener('click', function(){\n"
"      alive = false;\n"
"      noBtn.style.display = 'none';\n"
"      questionHeader.innerHTML = 'Yay! I knew it! &#x1F495;';\n"
"      funnyMsg.classList.add('hidden');\n"
"      startConfetti();\n"
"      launchHearts();\n"
"      doShake();\n"
"    });\n"
"  })();\n"
"  </script>\n"
"</body>\n"
"</html>\n";

/* ----------------------------------------------------------------
 *  Minimal HTTP response helpers
 * ---------------------------------------------------------------- */
static const char *const http_200_header =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=utf-8\r\n"
"Cache-Control: no-cache\r\n"
"Connection: close\r\n"
"Content-Length: %zu\r\n"
"\r\n";

static const char *const http_404_response =
"HTTP/1.1 404 Not Found\r\n"
"Content-Type: text/plain\r\n"
"Connection: close\r\n"
"\r\n"
"404 Not Found";

/* ----------------------------------------------------------------
 *  Simple request parser
 * ---------------------------------------------------------------- */
static bool read_request(int client_fd, char *method, size_t mlen,
                         char *path,   size_t plen)
{
    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer)-1, 0);
    if (n <= 0) return false;
    buffer[n] = '\0';
    if (sscanf(buffer, "%31s %511s", method, path) != 2) return false;
    return true;
}

/* ----------------------------------------------------------------
 *  Main server loop
 * ---------------------------------------------------------------- */
int main(void)
{
    const char *listen_ip = "127.0.0.1";
    const uint16_t listen_port = 8081;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(listen_port),
        .sin_addr   = {.s_addr = inet_addr(listen_ip)}
    };

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on http://%s:%d\n", listen_ip, listen_port);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char method[32] = {0};
        char path[512]  = {0};

        if (!read_request(client_fd, method, sizeof(method), path, sizeof(path))) {
            close(client_fd);
            continue;
        }

        bool serve_root = (strcmp(method, "GET") == 0) && (strcmp(path, "/") == 0);

        if (serve_root) {
            size_t clen = strlen(page_html);
            char header[256];
            snprintf(header, sizeof(header), http_200_header, clen);
            send(client_fd, header, strlen(header), 0);
            send(client_fd, page_html, clen, 0);
        } else {
            send(client_fd, http_404_response, strlen(http_404_response), 0);
        }

        close(client_fd);
    }

    close(listen_fd);
    return EXIT_SUCCESS;
}

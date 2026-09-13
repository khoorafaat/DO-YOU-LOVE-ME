💕 Do You Love Me? 🥺
A fun, viral single-page web app that asks the ultimate question — "Do you love me?"

Try clicking "No"... if you can! 😏

✨ Features
🥺 The Question — A beautiful glassmorphism card asks "Do you love me?"
🏃 Runaway No Button — The "No" button wobbles and dodges around the screen every time you hover or click it
🔢 Attempt Counter — Tracks how many times you tried to click "No"
😂 Funny Messages — Rotating sassy messages appear after a few attempts
📱 Touch Support — Works on mobile too!
🎉 Celebration Mode — Click "Yes" and get rewarded with confetti 🎊, floating hearts 💖, and a screen shake!
🌈 Animated Gradient Background — Smooth pink-to-orange shifting gradient
🪟 Glassmorphism UI — Modern frosted-glass card design
🖥️ Preview

┌──────────────────────────────┐
│                              │
│     Do you love me? 🥺      │
│       Attempts: 7            │
│       [  Yes 💖  ]           │
│                              │
│              [ No 😢 ] ← 🏃💨│
│                              │
│  Made with ❤ love by         │
│  ASHISH SINGHA MAHAPATRA     │
└──────────────────────────────┘
🚀 Getting Started
Prerequisites
GCC (or any C compiler)
A web browser
Build & Run
bash

# Clone the repo
git clone https://github.com/YOUR_USERNAME/DO-YOU-LOVE-ME.git
cd DO-YOU-LOVE-ME
# Compile
gcc -Wall -O2 -o do_you_love_me do_you_love_me.c
# Run the server
./do_you_love_me
Open your browser and go to http://127.0.0.1:8081 🎉

🛠️ Tech Stack
Layer	Technology
Backend	C (POSIX sockets)
Frontend	HTML, CSS, JavaScript
Server	Single-threaded HTTP
Fonts	Google Fonts (Poppins)
Yes, the entire app — server, HTML, CSS, and JS — lives in one single C file. No frameworks. No dependencies. Just vibes. 😎

📁 Project Structure

DO YOU LOVE ME ?/
├── do_you_love_me.c    # The entire app (server + frontend)
└── README.md           # You're reading this!
💡 How It Works
A tiny C HTTP server starts on 127.0.0.1:8081
It serves an inline HTML page with embedded CSS & JS
The "No" button is position: fixed on the viewport — it freely wobbles across the entire screen
Every hover or click on "No" moves it to a random position and increments the attempt counter
The button also auto-wanders every 2 seconds on its own
Clicking "Yes" triggers a celebration with confetti, hearts, and a screen shake 🎊
🤝 Contributing
Feel free to fork, star ⭐, and share this with someone special! 😉

📜 License
This project is open source and available under the 
MIT License
.

cd '/home/ashishsm/DO YOU LOVE ME ?'
git init
echo -e "do_you_love_me\n*.o" > .gitignore
git add .
git commit -m "🥺 Do You Love Me? - Made with love by ASHISH SINGHA MAHAPATRA"
gh repo create "DO-YOU-LOVE-ME" --public --source=. --push

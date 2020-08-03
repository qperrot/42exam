```
lsof -c a.out // check socket leaks
fuser -k [PORT]/tcp // close server socket
```
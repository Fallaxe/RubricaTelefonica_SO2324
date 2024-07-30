# RubricaTelefonica_SO2324

## server
To use the server you must have openssl installed.
navigate to /server/ directory and run "make" to compile the server \\
If your project doesn't compile you might run
``` markdown
cd server/
make deps
make
```
server's makefile has also **make run** and **make clean**

The first boot of server it will generate the settings.txt file with user and password(hashed)
You can reset user and password with
``` markdown
cd bin/
server -r
```
The server use cJSON library from his github repository

## client
To compile the client and use it, navigate to /client/ and run "make".
``` markdown
cd  client/
make
```
you can login immediately with
``` markdown
./server -a <userHere> <passwordHere>
```
client's makefile has also **make run** and **make clean**

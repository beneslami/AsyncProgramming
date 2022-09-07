gcc -g -c event_loop.c -o event_loop.o
gcc -g -c demo_app.c -o demo_app.o
gcc -g event_loop.o demo_app.o -o exe -lpthread
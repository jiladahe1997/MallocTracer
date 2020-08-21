# Introduction
MallocTracer is a tracer for malloc and free operation in C, especially for MCUs, like STM32、NXP、8051...

### Why this
As C don't have a GC(Garbage Collector), the developer must Malloc and Free memeory manually.However, There is an old Chinese saying: 
>People can not escape from doing wrong

It's very likely to forget Free a memory after Malloc it, and it will cause [Memory Leak](https://en.wikipedia.org/wiki/Memory_leak).

Although there are a lot of tools for resolving this problem, but most of them are only work in Linux or Windows. This one is for MCUs or [RTOS](https://en.wikipedia.org/wiki/Real-time_operating_system#:~:text=A%20real%2Dtime%20operating%20system,or%20shorter%20increments%20of%20time.).

### How it works
It's just use two tables to record your Malloc and Free operation, later you could print the two tables to see 
 - Where you called Malloc
 - The times you called Malloc and Free 
 - If you Free every memory

 ![example](example.png)

 # Usage
 
 ### From Source
 Just copy the two files to your project, and add them into your complie toolchain.

 ### From STM32CubeMX
 TODO:  https://community.st.com/s/feed/0D53W00000Cg0y8SAB

 
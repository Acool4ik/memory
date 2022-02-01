Castom allocator of memory. It repeats the actions of the usual "malloc" from stdlib in "C", but the only difference is that the memory will give out array in global space (located in .bss section), representing imitation dinamic memory ([heap]) in some virtual mashine. 

Code can be used as an example for introduction in so big theme as "allocation memory". Programm don't use syscalls, therefore the allocator may be considered as crossplatform.

In case a long work the programm need defragmentation. 

Command for compiling and run programm in Linux: `gcc -o Allocator Allocator.c && ./Allocator`


   [heap]: <https://ru.wikipedia.org/wiki/%D0%9A%D1%83%D1%87%D0%B0_(%D0%BF%D0%B0%D0%BC%D1%8F%D1%82%D1%8C)/>

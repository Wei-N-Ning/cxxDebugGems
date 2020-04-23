
this article seems useful:

https://stackoverflow.com/questions/7648642/how-to-use-the-addr2line-command-in-linux

Please check:

* Whether all the functions in your binary are compiled with -g, addr2line only support functions has debug information, that is compiled with -g

* Whether your offset is a valid offset. That means your offset should not be an virtual memory address, and should only be an offset in the .text section. In the .text section means the address should point to an instruction in the binary<Paste>


note this article "helped" me at work:

https://github.com/jcmvbkbc/binutils-gdb-xtensa/issues/5

it basically claims that GCC -ffunctions-section and -fdata-section 
affects addr2line 

I used this finding as an excuses to do some experiment, it shows:

- both optimisation does not change the performance much (either better or 
worse)

- it does not affect addr2line (ubuntu 16, gcc 4.8.5)



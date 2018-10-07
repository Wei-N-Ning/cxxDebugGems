
this article seems useful:

https://stackoverflow.com/questions/7648642/how-to-use-the-addr2line-command-in-linux

Please check:

* Whether all the functions in your binary are compiled with -g, addr2line only support functions has debug information, that is compiled with -g

* Whether your offset is a valid offset. That means your offset should not be an virtual memory address, and should only be an offset in the .text section. In the .text section means the address should point to an instruction in the binary<Paste>





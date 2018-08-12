
// source
// oroboro.com: printing stack traces file line
//
// only work with -g (-ggdb2)
// with -O0 your file and line info will be completely correct; if you 
// add -O3 many functions will be inlined, some code may be run out of 
// order etc... in this case the compiler will do its best to give you 
// correct file and line info but this may not always be possible
// (# recall in GDB, if the subject is O3 I often get "variable is 
// optimized out" warning, or simply can not step into a function at 
// all)

// TODO: 

int main() {
    return 0;
}

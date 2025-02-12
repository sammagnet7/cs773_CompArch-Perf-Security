##### creating shared library in task1
- gcc -g -fPIC -shared -o libshared.so shared.c 
##### find shared array location in elf executable
- nm -D libshared.so | grep shared_array
# LD_PRELOAD Rootkit

![made-with-C](https://shields.io/badge/Made_With-C-green?logo=Linux&style=for-the-badge)

## How To Compile It ?

```gcc smoochum.c -fPIC -shared -D_GNU_SOURCE -o libc.man.so.6 -ldl```

Let's break down the command :
- ```gcc``` : Our very own **GNU Compiler Collection**
- ```smoochum.c``` : The name of our program (Get the pokemon refernce ?)
- ```-fPIC``` : Generate position-independent code
- ```shared``` : Create a Shared Object which can be linked with other objects to produce an executable
- ```-D_GNU_SOURCE``` :  It is specified to satisfy ```#ifdef``` conditions that allow us to use the ```RTLD_NEXT``` enum. Optionally this flag can be replaced by adding ```#define _GNU_SOURCE``` 
- ```-o``` : Create an output file
- ```libc.man.so.6``` : Name of output file
- ```-ldl``` : Link against ```libdl```

## Functions Hooked :

- **ssize_t write(int fd, const void \*buf, size_t count)** : To Provide Reverse or Bind Shell as per trigger 
- **FILE \*fopen(const char \*pathname, const char \*mode);**  To Hide ```netstat``` and ```lsof``` connections 
- **struct dirent \*readdir(DIR \*dirp);** : To Hide our ```so``` file from ```ls```

***Note : The variants of these functions are provided incase the file sizes are large***

## To-Do :

- Add SSL Encryption
- Test and Debug IPv6 compatibilty
- Hide our Shared Object from ```ldd```
- **MORE !**

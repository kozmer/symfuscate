In this PoC, system & execve was used as a demonstration. As you see from the readelf command below, the symbol table does not include any of these.

```shell
$ readelf -Ws symfuscate

Symbol table '.dynsym' contains 19 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND dl_iterate_phdr@GLIBC_2.2.5 (2)
     2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND free@GLIBC_2.2.5 (2)
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.34 (3)
     4: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTable
     5: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.2.5 (2)
     6: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fread@GLIBC_2.2.5 (2)
     7: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fclose@GLIBC_2.2.5 (2)
     8: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __stack_chk_fail@GLIBC_2.4 (4)
     9: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND memset@GLIBC_2.2.5 (2)
    10: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    11: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND malloc@GLIBC_2.2.5 (2)
    12: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fseek@GLIBC_2.2.5 (2)
    13: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __printf_chk@GLIBC_2.3.4 (5)
    14: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fopen@GLIBC_2.2.5 (2)
    15: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND exit@GLIBC_2.2.5 (2)
    16: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
    17: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@GLIBC_2.2.5 (2)
    18: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND strstr@GLIBC_2.2.5 (2)
```

PoC Output:
```shell
$ ./symfuscate

[i] Searching in: /usr/lib/libc.so.6
[i] Found dynsym @ 0x54b8 and dynstr @ 0x17c68
[i] Number of symbols: 3154
[+] Found 'system' in /usr/lib/libc.so.6 @ 0x7d49d91acf10
[+] Found 'execve' in /usr/lib/libc.so.6 @ 0x7d49d923cfb0
[+] Resolved function @ i[0]: 0x7d49d91acf10
[+] Resolved function @ i[1]: 0x7d49d923cfb0
Linux
CMakeCache.txt  CMakeFiles  Makefile  cmake_install.cmake  symfuscate
```
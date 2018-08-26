# C++ Vector serialization benchmark

Boost version 1.62.0. Compiled with gcc/g++ 6.3.0 with -O3 optimizations.

Executed on an Intel T2310 CPU @1.45GHz.

## boost::archive performances

Performances of boost's binary_archive, text_archive, xml_archive.

1M random elements, in full type range for integral types, between -1 and 1 for floating point types.

|                |                 |  char  |  short |  int |  long  |  float | double |
|----------------|-----------------|:------:|:------:|:----:|:------:|:------:|:------:|
|                |   serialization |   3ms  |   8ms  | 16ms |  34ms  |  98ms  |  348ms |
| binary_archive | deserialization |   2ms  |  38ms  |  5ms |  11ms  |  88ms  |  128ms |
|                |    archive size |   1MB  |   2MB  |  4MB |   8MB  |   4MB  |   8MB  |
|                |   serialization |  160ms |  175ms | 275ms|  378ms |   1s   |  1.2s  |
| text_archive   | deserialization |  219ms |  261ms | 443ms|  684ms |  1.2s  |  1.6s  |
|                |    archive size |  3.6MB |  5.8MB | 11MB | 20.3MB | 16.5MB | 25.5MB |
|                |   serialization |  936ms |  928ms |  1s  |  1.2s  |   2s   |  2.2s  |
| xml_archive    | deserialization |  959ms |   1s   | 1.2s |  1.4s  |  2.3s  |  2.7s  |
|                |    archive size | 17.6MB | 19.8MB | 25MB | 34.4MB | 30.5MB | 38.5MB |

## Base64 encoding

|                    |                 | char | short |  int  |  long | float | double |
|--------------------|-----------------|:----:|:-----:|:-----:|:-----:|:-----:|:------:|
| Boost raw base64   |   serialization | 18ms |  38ms |  83ms | 180ms |  98ms |  179ms |
|                    | deserialization | 31ms |  63ms | 127ms | 253ms | 125ms |  251ms |
| Boost typed base64 |   serialization | 19ms |  34ms |  70ms | 191ms |  N/A  |   N/A  |
|                    | deserialization | 30ms |  56ms |  99ms | 196ms |  N/A  |   N/A  |
| Coreutils          |   serialization |  5ms |  11ms |  23ms |  58ms |  23ms |  58ms  |
|                    | deserialization |  7ms |  14s  |  27ms |  57ms |  27ms |  68ms  |

## License

The base64.c and base64.h files are part of [*coreutils*](https://www.gnu.org/software/coreutils/coreutils.html) and are licensed under the GPLv2 license.

They have been modified to:

- remove the include of the `config.h` file, which is specific to coreutils.
- disable C++ mangling when used in a C++ project.
- selectively inhibit the `restrict` keyword, which is not supported by C++ compilers.

The rest of the project is released under the MIT license.


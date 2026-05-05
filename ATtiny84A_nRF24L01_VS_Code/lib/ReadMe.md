The `lib` directory is intended for project specific (private) libraries.


The source code of each library should be placed in an own separate directory

---

For example, see a structure of the following two libraries `Foo` and `Bar`:

```
|--lib
|  |
|  |--Bar
|  |  |- Bar.c
|  |  |- Bar.h
|    
|  |--Foo
|  |  |- Foo.c
|  |  |- Foo.h
|  |
|  |- ReadMe.md --> THIS FILE
|
|--include
|  |- main.h
|
|--src
|  |- main.c
```

---


`main.c`

```c
#include "main.h"
#include "Foo.h"
#include "Bar.h"

int main (void)
{
  ...
}

```


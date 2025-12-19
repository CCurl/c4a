( Block #002 )

." %Wc4a - %Gv" .version cr
 mem-sz       ." %YMemory: %W%d bytes.%n"
 here code-sz ." %Y  Code: %W%d slots, %d used.%n"
 last vhere -  vhere vars -
              ." %Y  Heap: %W%d bytes used, %d free.%n"
 dict-end last -  de-sz /
              ." %Y Words: %W%d defined."
 de-sz        ."  %d bytes per entry.%n"














leoimginfo
============================
2015 jkbenaim

Show information about your 64DD disk images.

## Example run
```console
> leoimginfo NUD-EFZJ-JPN.bin
SYSTEM AREA
  Disk type     : 3 (retail)
  IPL load address : 80000400
  IPL load size : 38
  ROM end lba   : 2991
  RAM start lba : 3062
  RAM end lba   : 3128
DISK ID
  Initial code  : EFZJ
  Game version  : 0
  Disk number   : 0
  RAM use       : yes
  disk use      : 0
  timestamp     : 2000-04-05 11:45:20
  company code  : 01
  free area     : 4e4d412d4558
SHA1 of ROM area: 811A6197BA132333D0F3FF8516F5258EF0B3CE2D

```

See the original readme in readme-orig.txt.

## Building

Just run ```make ```.

## License

sha1.c and sha1.h are both in the public domain. Other files are GPLv3.
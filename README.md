leoimginfo
============================
2015 jkbenaim

Show information about your 64DD disk images, and extract MFS contents (if they exist).

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
MFS (RAM)
  ram start off : 30b7180
  present       : yes (0)
  attr          : 20 (--R)
  type          : 3 (match)
  volume name   : 
  format date   : 1999-01-01 00:00:00
  renewal count : 185
  destination   : Japan
  checksum      : EEBCCAD0
  max files     : 644

     dir:       .
     dir:       ./F-ZEROX
   20398:       ./F-ZEROX/CAPTAIN FALCON.MA2D1
   13049:       ./F-ZEROX/Dr．STEWART.MA2D1
   21391:       ./F-ZEROX/PICO.MA2D1
   26222:       ./F-ZEROX/SAMURAI GOROH.MA2D1
   29213:       ./F-ZEROX/JODY & JOHN.MA2D1
   22485:       ./F-ZEROX/MM GAZELLE.MA2D1
   18707:       ./F-ZEROX/BABA.MA2D1
   17072:       ./F-ZEROX/OCTMAN.MA2D1
   22401:       ./F-ZEROX/Dr．CLASH.MA2D1
   24937:       ./F-ZEROX/Mr．EAD.MA2D1
   23693:       ./F-ZEROX/BIO REX.MA2D1
   20151:       ./F-ZEROX/SILVER NEELSEN.MA2D1
   23809:       ./F-ZEROX/GOMAR & SHIOH.MA2D1
   34162:       ./F-ZEROX/SUPER & Mrs．ARROW.MA2D1
   19369:       ./F-ZEROX/BLOOD FALCON.MA2D1
   14153:       ./F-ZEROX/JACK LEVIN.MA2D1
   15667:       ./F-ZEROX/JAMES McCLOUD.MA2D1
   18389:       ./F-ZEROX/ZODA.MA2D1
   17278:       ./F-ZEROX/MICHAEL CHAIN.MA2D1
   13651:       ./F-ZEROX/KATE ALEN.MA2D1
   20252:       ./F-ZEROX/ROGER BUSTER.MA2D1
   17743:       ./F-ZEROX/LEON.MA2D1
   28987:       ./F-ZEROX/DRAQ.MA2D1
   17551:       ./F-ZEROX/BEASTMAN.MA2D1
   17984:       ./F-ZEROX/ANTONIO GUSTER.MA2D1
   17623:       ./F-ZEROX/BLACK SHADOW.MA2D1
   17899:       ./F-ZEROX/ARBIN GORDON.MA2D1
   20741:       ./F-ZEROX/BILLY.MA2D1
      32:       ./F-ZEROX.CARD
      32:       ./JPN.CARD
      32:       ./427.CARD
      24:       ./OPTION.OPT
      32:       ./SOE.CARD
      32:       ./NINTENDO.CARD
   51248:       ./GOKURAKU.CRSD
      32:       ./ZR-F.CARD
   51248:       ./GIGOKU.CRSD
   51248:       ./IONNGU.CRSD
   51248:       ./GGKR-D.CRSD
      32:       ./JAPAN.CARD
      32:       ./NARUTO.CARD
      32:       ./LUKE.CARD
   51248:       ./MISAE.CRSD
   51248:       ./DAFFNNDA.CRSD
   51248:       ./HOGE.CRSD
   51248:       ./GHOST30.GOST
   51248:       ./BLEACH.CRSD
      32:       ./GOK-78.CARD
      32:       ./DIEGO.CARD
   51248:       ./OHSHIT.CRSD
      32:       ./FUCKER87.CARD
     216:       ./CRS_ENTRY.CENT


```

## Building

Just run ```make ```.

## License

sha1.c and sha1.h are both in the public domain. Other files are GPLv3.
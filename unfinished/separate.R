# -*- makefile -*-

SEPARATE_EXTRA = divvy dsf

separate       : [X] GTK COMMON separate SEPARATE_EXTRA separate-icon|no-icon

separate       : [G] WINDOWS COMMON separate SEPARATE_EXTRA separate.res|noicon.res

ALL += separate[COMBINED] SEPARATE_EXTRA

!begin gtk
GAMES += separate
!end

!begin >list.c
    A(separate) \
!end

!begin >wingames.lst
separate.exe:Separate
!end

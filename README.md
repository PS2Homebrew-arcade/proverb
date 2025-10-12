# Proverb

Proverb is an alternative bootloader for Arcade PS2s (COH-H model series and their direct derivative units)

the idea behind this is to use proverb as a fixed `boot.bin` file, then have whatever program you want to boot first as a plain ELF wich you can update more comfortably.

also, it could be used to reduce boot times on original security dongle's from games


## Usage
the program can be built to look for the elf file to run from a config file. but it can also be hardcoded, specified at compile time, like the psalm bootloader from sony

## FAQ

- Why is it called Proverb?

> Because the official sony bootloader was part of the sony "Psalm" library. and since Proverb is the first bible book after Psalm, I thought it was neat to use that as name.

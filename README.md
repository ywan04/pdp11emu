# pdp11emu

PDP-11 minicomputer emulator

## build

```bash
git clone https://github.com/ywan04/pdp11emu
cd pdp11emu
make
```

## usage

run a program:
```bash
./pdp11emu f filename
```

for help use:
```bash
./pdp11emu h
```

## file format

The first line contains a single decimal number *n* - the number of load operations to be performed. The next *n* lines contain two octal numbers each, the address and the data to be loaded into memory at that address.
Example:
```
7
001000 105737
001002 177564
001004 100375
001006 112737
001010 000101
001012 177566
001014 000771
```

## license

[MIT](https://github.com/ywan04/pdp11emu/blob/master/LICENSE)
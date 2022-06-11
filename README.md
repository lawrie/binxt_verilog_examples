# Verilog examples for the Blackice Nxt ice40 FPGA board

These tests run on [Blackice Nxt](https://github.com/folknology/BlackIceNxt).

## menu

This is a test of using a blade to access an SD card from the FPGA.

It is a picorv32 Soc that reads the root directory of a FAT32 SD card and displays the file names as a menu on the VGA output.

It needs a VGA tile in tile 1 position, an LED blade in blade 1, and the SD card in blade 2.

The SD card should be freshly formattted with 8 files in 8.3 filename format in the root directory.

There is an optional external uart connected to pmod 5 (in the tile 2 position).

The uart gives some diagnostics and also displays the file names. It is at 9600 baud.

The test can be sensitive to the position of the SD card in the blade.

## 7seg

This is a test of the 7-segment tile in position 3.

The test counts in hex, incrementing a digit approximately once per second.

#!/bin/env bash
esptool.py --chip esp32 --port $1 --baud 230400 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 0x1000_Navinaut-AIS.bootloader.bin 0x8000 0x8000_Navinaut-AIS.partitions.bin 0xe000 0xe000_Navinaut-AIS.boot_app0.bin 0x10000 0x10000_Navinaut-AIS.bin

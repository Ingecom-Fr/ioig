#!/usr/bin/python3

import re
import subprocess

command = "lsusb | grep 'ioig Multi Protocol Dongle'"

# Launch the command lsusb | grep 'TinyUSB'
lsusb_output = subprocess.check_output(command, shell=True, text=True)

#print(lsusb_output)

# Regex to extract the Bus and Device numbers
regex = r"Bus\s+(\d+)\s+Device\s+(\d+)"

# Find all matches in the lsusb_output
matches = re.findall(regex, lsusb_output)


# Print the string line for each match
for match in matches:
    usb_bus = match[0]
    usb_device = match[1]
    usb_bus = usb_bus.lstrip("0")
    usb_device = usb_device.lstrip("0")

    print(f"Bus {usb_bus} Device {usb_device}")
    print(f"usb.src matches \"({usb_bus}\.{usb_device}\.[0-9])\" || usb.dst matches \"({usb_bus}\.{usb_device}\.[0-9])\"")    
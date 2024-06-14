#!/bin/bash
clear ;\
make  && \
echo "Build done!" &&  \
[ -d "/media/$USER/RPI-RP2/" ] && \
ls -alF fw/ioig_fw.uf2 &&  \
cp fw/ioig_fw.uf2 /media/$USER/RPI-RP2/ && \
echo "flash done!"
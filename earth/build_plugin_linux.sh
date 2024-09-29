#!/bin/bash

mkdir LinuxBinaries

TOP=$(pwd)  

chmod +x build_linux.sh

./build_linux.sh

cp $(find . -name 'earth.so') $TOP/LinuxBinaries

cd .. 

cd port_gradient_map && chmod +x build_linux.sh

./build_linux.sh

cp $(find . -name 'gradient-map-port.so') $TOP/LinuxBinaries


#!/bin/bash

mkdir WindowsBinaries

TOP=$(pwd)  

chmod +x build_linux.sh

./build_linux.sh

cp $(find . -name 'earth.dll') $TOP/WindowsBinaries

cd .. 

cd port_gradient_map && chmod +x build_linux.sh

./build_linux.sh

cp $(find . -name 'gradient-map-port.dll') $TOP/WindowsBinaries


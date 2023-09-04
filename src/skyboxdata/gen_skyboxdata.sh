#!/bin/bash

cp skybox.jpg skyboxdata
echo "#include \"skyboxdata.h\"" > skyboxdata.cpp
echo >> skyboxdata.cpp
xxd -i skyboxdata >> skyboxdata.cpp
sed -i 's/unsigned/const unsigned/g' skyboxdata.cpp
rm skyboxdata

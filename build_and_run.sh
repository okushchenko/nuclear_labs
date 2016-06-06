#!/bin/bash
set -x
set -e

cur_dir="$PWD"
geant_version="10.2.1"
lab_name="B1"
geant_dir="/Users/oleksa/univ/Geant4-${geant_version}-Darwin"

if [ -d "${cur_dir}/${lab_name}-build" ]; then
    rm -r "${cur_dir}/${lab_name}-build"
fi
mkdir "${cur_dir}/${lab_name}-build"
cd "${cur_dir}/${lab_name}-build"
cmake -DGeant4_DIR="${geant_dir}/lib/Geant4-${geant_version}" "../${lab_name}"
make -j1
make install

cd "${geant_dir}/share/Geant4-${geant_version}/geant4make"
source ./geant4make.sh
cd "${cur_dir}/${lab_name}-build"
/usr/local/bin/exampleB1
cd ${cur_dir}

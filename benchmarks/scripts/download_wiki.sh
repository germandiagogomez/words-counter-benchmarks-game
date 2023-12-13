#!/usr/bin/env bash
set -euox pipefail

SCP=$1
XZ=$2
TAR=$3
OUTPUT0=$4
OUTPUT1=$5
SOURCE_ROOT=$6

if [[ ! -f "${SOURCE_ROOT}/wikipedia_resources.tar.bz2" ]]; then
  echo 'Downloading wikipedia_resources.tar.bz2 from cppmaster.net...'
  trap "rm ${SOURCE_ROOT}/wikipedia_resources.tar.bz2" SIGINT
  $SCP -P 2221 bigstorage@cppmaster.net:wikipedia_resources.tar.bz2 ${SOURCE_ROOT}/wikipedia_resources.tar.bz2
  touch $OUTPUT0
fi


OLD_DIR=$PWD
trap "cd ${OLD_DIR}" EXIT
echo 'Uncompressing wikipedia_resources.tar.bz2...'
$TAR xjvf ${SOURCE_ROOT}/wikipedia_resources.tar.bz2
cp -r wikipedia_resources ${SOURCE_ROOT}/wikipedia_resources
rm ${SOURCE_ROOT}/wikipedia_resources.tar.bz2

echo "Done. Your resources are in ${SOURCE_ROOT}/wikipedia_resources."
touch $OUTPUT1

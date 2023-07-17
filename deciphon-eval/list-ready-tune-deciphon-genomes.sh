#!/bin/bash

root=$1
for i in $(find $root -maxdepth 4 -type f -name "cds_from_genomic.dcs" -exec dirname {} \;); do
  realpath "$i"
done

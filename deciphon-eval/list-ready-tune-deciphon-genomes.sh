#!/bin/bash

root=/nfs/research/rdf/horta/tune-deciphon/phase3
for i in $(find $root -type f -name "cds_from_genomic.dcs" -exec dirname {} \;); do
  realpath "$i"
done

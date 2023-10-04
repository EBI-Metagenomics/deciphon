#!/bin/bash

root=$1
pfam_file=$root/Pfam-A.hmm
genome_dirs=$(find $root -maxdepth 4 -type f -name "cds_from_genomic.dcs" -exec dirname {} \;)

for genome_dir in $genome_dirs; do
  cmd="python script/guessed_positive_nucleotides.py $genome_dir $pfam_file"
  bsub -g /horta/eval "$cmd"
done

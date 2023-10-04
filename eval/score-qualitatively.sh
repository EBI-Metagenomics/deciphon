#!/bin/bash

root=$1
pfam_file=$root/Pfam-A.hmm
genome_dirs=$(find $root -maxdepth 4 -type f -name "cds_from_genomic.dcs" -exec dirname {} \; | tr '\n' ' ')

python script/score_qualitatively.py $root/qualitatively.parquet $pfam_file $genome_dirs
# cmd="python script/score_qualitatively.py $root/qualitatively.parquet $pfam_file $genome_dirs"
# bsub -g /horta/eval "$cmd"

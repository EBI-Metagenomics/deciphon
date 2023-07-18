#!/bin/bash

db=/Users/horta/tune-deciphon/db.hmm
root=/Users/horta/tune-deciphon/phase4

genome_dirs() {
  for i in $(find $root/$1 -type f -name "*.dcs" -exec realpath {} \;); do
    dirname $i
  done
}

for i in 0 m10; do
  python script/score_quantitatively_whole_genome.py quant_$i.parquet $db $(genome_dirs $i)
done

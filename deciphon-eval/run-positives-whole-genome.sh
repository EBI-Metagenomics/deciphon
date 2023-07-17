#!/bin/bash

db=/nfs/research/rdf/horta/tune-deciphon/db.hmm

for i in $(./list-ready-tune-deciphon-genomes.sh); do
  bsub -g /horta/evaluate "python script/positives_whole_genome.py $i $db -s"
done

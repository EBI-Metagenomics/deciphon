#!/bin/bash

db=/Users/horta/tune-deciphon/db.hmm
root=/Users/horta/tune-deciphon/phase4

for i in $(./list-ready-tune-deciphon-genomes.sh $root); do
  echo $i
  python script/positives_qualitatively.py $i $db
done

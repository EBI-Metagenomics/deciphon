#!/bin/sh

test -f minifam.hmm.pid && h3daemon stop minifam.hmm; rm -f minifam.hmm.*
test -f three.hmm.pid   && h3daemon stop three.hmm  ; rm -f three.hmm.*

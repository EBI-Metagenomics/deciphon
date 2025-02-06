#!/bin/sh
set -e

h3daemon stop "$1"
test -e $1.h3f && rm $1.h3f
test -e $1.h3i && rm $1.h3i
test -e $1.h3m && rm $1.h3m
test -e $1.h3p && rm $1.h3p
test -e $1.pid && rm $1.pid

#!/bin/sh

test -f $1.pid && h3daemon stop $1; rm -f $1.*

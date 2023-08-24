#!/bin/sh
set -x

pipx run h3daemon start "$1" --port 51371 --force \
    --stdout h3daemon.stdout \
    --stderr h3daemon.stderr || exit 1

if ! pipx run h3daemon isready "$1" --wait
then
    cat h3daemon.stdout h3daemon.stderr
    exit 1
fi

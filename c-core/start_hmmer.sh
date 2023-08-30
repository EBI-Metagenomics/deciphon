#!/bin/sh
set -x

cleanup() {
  rv=$?
  if [ $rv -ne 0 ]
  then
    cat h3daemon.stdout h3daemon.stderr
  fi
  rm -rf h3daemon.stdout h3daemon.stderr || true
  exit $rv
}
trap "cleanup" EXIT

pipx run h3daemon start "$1" --port 51371 --force \
    --stdout h3daemon.stdout \
    --stderr h3daemon.stderr || exit $?

pipx run h3daemon isready "$1" --wait

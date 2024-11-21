#!/bin/sh

command -v h3daemon >/dev/null || pipx install h3daemon[cli]

h3daemon start $1 --port $2 >/dev/null
h3daemon ready $1 --wait >/dev/null

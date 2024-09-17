#!/bin/sh

command -v h3daemon >/dev/null || pipx install h3daemon

h3daemon start minifam.hmm --port 51300 >/dev/null
h3daemon ready minifam.hmm --wait >/dev/null

h3daemon start three.hmm --port 51301 >/dev/null
h3daemon ready three.hmm --wait >/dev/null

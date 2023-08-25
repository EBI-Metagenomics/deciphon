# Welcome to Deciphon projects 👋

The main part of Deciphon is its core. It is implemented in C and built on top of [IMM](https://github.com/EBI-Metagenomics/imm) library to define protein using hidden Markov models and perform inference using the Viterbi method.

## Quick start

Its command-line interface can be installed by

```sh
pip install deciphon
```

if you have a working Python environment. It will run on MacOS and Linux operating systems. After installing it, just enter

```sh
deciphon --help
```

in the terminal to show usage information.

## Directory layout

    ├─ c-core/          Deciphon library written in C.
    ├─ cli/             Command-line interface written in Python.
    ├─ control/         Command-line interface to control and run the servers.
    ├─ eval/            Python scripts to evaluate its performance.
    ├─ python-core/     Python wrapper around the Deciphon library.
    ├─ sched/           RESTful API for Deciphon server in Python.
    ├─ snap/            Reader for Deciphon snap files written in Python.
    ├─ tests/           Unit tests.

## Compose

The relevant files are `compose.yml` and `compose.cfg`. You can launch it via `docker compose --env-file compose.cfg up` and test it running `./tests/test-compose.sh`. Once it is running, open `http://127.0.0.1:1515/docs` to have an overview of the RESTful API.

You might want to change the variable `DECIPHON_SCHED_ALLOW_ORIGINS` in the file `compose.cfg` to the hostname of a web server.

## 👤 Author

- [Danilo Horta](https://github.com/horta)

## Show your support

Give a ⭐️ if this project helped you!

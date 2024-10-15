<h1 align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="./.github/assets/logo-dark.png">
    <source media="(prefers-color-scheme: light)" srcset="./.github/assets/logo-light.png">
  </picture>
</h1>

Deciphon is a robust protein annotation software tailored for handling long-read
sequencing data with high error rates. By leveraging a novel approach that
incorporates quasi-codons, Deciphon allows for direct annotation of protein sequences
without the need for prior error correction or open reading frame identification.

<p align="center">
    <img src="./img/chlamydia.png" alt="Comparison on Chlamydia long-read" width="740">
</p>

The figure above illustrates the Pfam annotations found using Deciphon, compared to combinations
of Prodigal and FragGeneScan with HMMER, on a long-read of the Chlamydia strain 14-2711_R47,
isolated from flamingos.
The alignment spans a chromosomal region of 23,953 nucleotides and consists of 23,242 matches,
880 deletions, and 711 insertions. Deciphon recovered 21 out of 29 proteins (72%) while
mislabelling only one (Chordopox_A30L). FGS1-HMMER and FGS3-HMMER recovered 10 (34%) and
8 (28%) proteins, respectively. Prodigal-HMMER predicted 9 genes but found no significant
match against Pfam.

## Quick start

To get started, you can install Deciphon via the command line using Python’s package manager:

```sh
pip install deciphon
```

Deciphon is compatible with both __macOS__ and __Linux__ operating systems.
Once installed, you can access the help documentation by entering:

```sh
deciphon --help
```

in the terminal to display usage information.

## Directory layout

    ├─ c-core/          Deciphon core library written in C.
    ├─ cli/             Command-line interface (CLI) written in Python.
    ├─ compose/         Docker Compose setup for running a Deciphon server.
    ├─ control/         CLI for controlling and managing servers.
    ├─ intervals/       Python and R-based interval definitions.
    ├─ python-core/     Python wrapper for the Deciphon core library.
    ├─ sched/           RESTful API for Deciphon server.
    ├─ snap/            Python reader for Deciphon snap files.

## 👤 Author

- [Danilo Horta](https://github.com/horta)

## Show your support

If you find this project useful, please consider giving it a ⭐️!

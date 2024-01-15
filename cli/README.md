# Welcome to deciphon 👋

> Individually annotate long, error-prone nucleotide sequences into proteins

### 🏠 [Homepage](https://github.com/EBI-Metagenomics/deciphon-py)

## ⚡️ Requirements

- Python >= 3.9
- Pip
- [Podman](https://podman.io) >= 3.4
- [Homebrew](https://brew.sh) on MacOS (recommended)
- [Pipx](https://pypa.github.io/pipx/) for Python package management (recommended)

### MacOS

Install Python and Podman:

```sh
brew update && brew install python podman pipx
```

Ensure that your `PATH` environment variable is all set:

```sh
pipx ensurepath
```

💡 You might need to close your terminal and reopen it for the changes to take effect.

### Ubuntu (and Debian-based distros)

Install Python and Podman:

```sh
sudo apt update && \
    sudo apt install python3 python3-pip python3-venv podman --yes && \
    python3 -m pip install --user pipx
```

Ensure that your `PATH` environment variable is all set:

```sh
python3 -m pipx ensurepath
```

💡 You might need to close your terminal and reopen it for the changes to take effect.

## Install

```sh
pipx install deciphon
```

## Usage

```
 Usage: deci [OPTIONS] COMMAND [ARGS]...

╭─ Options ────────────────────────────────────────────────────────────────────╮
│ --version                                                                    │
│ --help             Show this message and exit.                               │
╰──────────────────────────────────────────────────────────────────────────────╯
╭─ Commands ───────────────────────────────────────────────────────────────────╮
│ press        Press HMM ASCII file into a Deciphon database one.              │
│ scan         Annotate nucleotide sequences into proteins a protein database. │
╰──────────────────────────────────────────────────────────────────────────────╯
```

## Example

Download the `minifam.hmm` protein database:

```sh
curl -O https://raw.githubusercontent.com/EBI-Metagenomics/deciphon/main/cli/tests/files/minifam.hmm
```

Download the `consensus.fna` file of sequences:

```sh
curl -O https://raw.githubusercontent.com/EBI-Metagenomics/deciphon/main/cli/tests/files/sequences.fna
```

Press it (using The Standard Code):

```sh
deci press minifam.hmm 1
```

Scan it:

```sh
deci scan minifam.hmm sequences.fna
```

Show it:

```sh
deci see sequences.dcs
```

## 👤 Author

- [Danilo Horta](https://github.com/horta)

## Show your support

Give a ⭐️ if this project helped you!

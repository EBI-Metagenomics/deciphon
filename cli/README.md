# Welcome to deciphon 👋

> Individually annotate long, error-prone nucleotide sequences into proteins

### 🏠 [Homepage](https://github.com/EBI-Metagenomics/deciphon-py)

## ⚡️ Requirements

- Python >= 3.11
- Pip
- [Homebrew](https://brew.sh) on MacOS (recommended)
- [Pipx](https://pypa.github.io/pipx/) (or [uvx](https://docs.astral.sh/uv/guides/tools/)) for Python application management (recommended)

### MacOS

Install Python and Pipx:

```sh
brew update && brew install python pipx
```

Ensure that your `PATH` environment variable is all set:

```sh
pipx ensurepath
```

💡 You might need to close your terminal and reopen it for the changes to take effect.

### Ubuntu (and Debian-based distros)

Install Python:

```sh
sudo apt update && \
    sudo apt install python3 python3-pip python3-venv --yes && \
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
 Usage: deciphon [OPTIONS] COMMAND [ARGS]...

╭─ Options ────────────────────────────────────────────────────────────────────╮
│ --version          Show version.                                             │
│ --help             Show this message and exit.                               │
╰──────────────────────────────────────────────────────────────────────────────╯
╭─ Commands ───────────────────────────────────────────────────────────────────╮
│ press    Make protein database.                                              │
│ scan     Scan nucleotide sequence against protein database.                  │
│ see      Display scan results.                                               │
│ worker   Launch worker.                                                      │
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
deciphon press minifam.hmm 1
```

Scan it:

```sh
deciphon scan minifam.hmm sequences.fna
```

Show it:

```sh
deciphon see sequences.dcs
```

## 👤 Author

- [Danilo Horta](https://github.com/horta)

## Show your support

Give a ⭐️ if this project helped you!

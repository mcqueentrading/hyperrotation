# HyprRotation

Simple Hyprland monitor rotation helper for quickly applying a transform across connected displays from the command line.

## Overview

This project provides a small shell wrapper around `hyprctl` so monitor rotation can be switched without editing configuration files by hand.

Supported modes:

- `normal`
- `right`
- `down`
- `left`
- `lef`

The current live entrypoint is:

- `hypr_rotation.sh`

Older Python versions are kept in `legacy/` for reference.

## Quick Start

```bash
chmod +x hypr_rotation.sh
./hypr_rotation.sh normal
./hypr_rotation.sh right
./hypr_rotation.sh down
./hypr_rotation.sh left
```

## Shell Setup

Place the script somewhere stable, for example:

```bash
mkdir -p "$HOME/.local/bin"
cp hypr_rotation.sh "$HOME/.local/bin/hypr_rotation.sh"
chmod +x "$HOME/.local/bin/hypr_rotation.sh"
```

Then add aliases to your shell config such as `~/.bashrc`:

```bash
alias nom="$HOME/.local/bin/hypr_rotation.sh normal"
alias rot="$HOME/.local/bin/hypr_rotation.sh right"
alias down="$HOME/.local/bin/hypr_rotation.sh down"
alias lef="$HOME/.local/bin/hypr_rotation.sh lef"
alias left="$HOME/.local/bin/hypr_rotation.sh left"
```

Reload your shell after editing:

```bash
source ~/.bashrc
```

## Requirements

- `bash`
- `hyprctl`
- a running Hyprland session

## How It Works

The shell script:

1. validates the requested rotation mode
2. discovers connected monitors via `hyprctl monitors`
3. applies the matching Hyprland transform to each detected monitor
4. prints a simple result summary

## Repository Scope

This repo is intentionally small.

- `hypr_rotation.sh` is the current live version
- `legacy/` contains older Python-based approaches that edited config files directly

## Project Page

Portfolio page:

https://tonimcqueen.com/project_hyprrotation.html

## License

MIT

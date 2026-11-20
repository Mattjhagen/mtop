# mtop

A btop 1.3.0 fork with a native UPS panel beneath the process list. The process list is shortened to make room; CPU, memory, disks, and network keep their full layout. The UPS panel uses btop's active theme, borders, background, and meter gradients.

Based on [aristocratos/btop](https://github.com/aristocratos/btop/tree/v1.3.0), Apache-2.0. See [upstream documentation](README.btop.md) and [LICENSE](LICENSE). The mtop changes are AI generated.

## Install with APT

The project runs a signed APT repository for Debian 12 and 13 and Ubuntu 24.04
on amd64 and arm64. See the [APT installation instructions](docs/apt.md) for
setup and key verification.

## Build and install on Ubuntu

```sh
sudo apt-get install build-essential nut-client
make -j2 GPU_SUPPORT=false
mkdir -p ~/.local/bin
install -m755 bin/btop ~/.local/bin/mtop-native
install -m755 mtop/mtop ~/.local/bin/mtop
```

Launch with `~/.local/bin/mtop`, or `mtop` when `~/.local/bin` is on PATH. No tmux is required for this launcher. The distribution's `/usr/bin/btop` is unchanged.

## UPS readings

The read-only panel queries `minuteman@localhost` through `/usr/bin/upsc` every five seconds with a two-second timeout. This name currently matches the R510's NUT configuration; edit `src/mtop_ups.hpp` and rebuild for a different UPS name.

It shows status, estimated runtime, battery charge, load, and available electrical readings. Implausible and unreadable fields are omitted. In particular, the current Minuteman/Delta HID driver reports zero voltages, 900 Hz, and 230 A; these are excluded, not rescaled. Plausibility bounds are tailored to this 120 V, 1000 VA UPS and do not prove that retained readings match its LCD. Wattage is labeled “reported”; runtime is an estimate, not a shutdown countdown. No driver or shutdown policy is changed.

The UPS panel is hidden if the process region has fewer than 25 rows. Scroll the shortened process list normally to see remaining processes. No processes are stopped or filtered by this layout.

## R510 attached-screen boot setup

`mtop/enable-r510-mtop-boot.sh` is specific to hostname `r510`, account `matt`, and its zsh login shell. It enables password-free login on local console 1, opens mtop automatically, and selects that console after the display manager starts. Physical access to this console gives access to the `matt` account. Other consoles and SSH login behavior are unchanged.

Review and run on the R510:

```sh
sudo bash mtop/enable-r510-mtop-boot.sh
```

The installer preserves backups of replaced configuration files and starts the display without rebooting. On the R510 already using the combined tmux launcher, `~/.local/bin/btop-ups` remains a compatibility launcher; `mtop` opens the same dashboard directly.

To undo boot startup, remove `/etc/systemd/system/getty@tty1.service.d/mtop.conf`, disable `mtop-console.service`, remove the marked R510 block from `~/.zprofile`, and run `sudo systemctl daemon-reload`. Restore backed-up configuration if it existed. Do not restart the active console until ready to close its session.

## Validation

Built on the R510 with Ubuntu 24.04 and GCC. Checked native rendering in a terminal at the user's 102×55 layout, UPS status/runtime/meters, omission of invalid telemetry, process-list space reservation, and small-terminal behavior. Boot configuration is staged separately and needs administrator execution; a reboot has not been tested.

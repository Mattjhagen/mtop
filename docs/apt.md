# Install mtop with APT

The mtop project hosts this third-party repository for Debian 12 and 13 and
Ubuntu 24.04 on amd64 and arm64.

Install the tools needed to download and verify the repository key.

```sh
sudo apt update
sudo apt install ca-certificates curl gnupg
```

Download the key and verify its primary fingerprint
`25602826F2BF7FF5893C7627DF90A98A5EA8B59A` before adding the repository.

```sh
(
  set -eu
  key=$(mktemp)
  trap 'rm -f "$key"' EXIT
  curl -fsSL https://mattjhagen.github.io/mtop/apt/mtop-archive-keyring.asc -o "$key"
  fingerprint=$(gpg --batch --show-keys --with-colons "$key" |
    awk -F: '$1 == "pub" { primary=1; next } primary && $1 == "fpr" { print $10; primary=0 }')
  test "$fingerprint" = 25602826F2BF7FF5893C7627DF90A98A5EA8B59A
  sudo install -d -m 0755 /etc/apt/keyrings
  sudo install -m 0644 "$key" /etc/apt/keyrings/mtop-archive-keyring.asc
  sudo tee /etc/apt/sources.list.d/mtop.sources >/dev/null <<SOURCES
Types: deb
URIs: https://mattjhagen.github.io/mtop/apt/
Suites: stable
Components: main
Architectures: $(dpkg --print-architecture)
Signed-By: /etc/apt/keyrings/mtop-archive-keyring.asc
SOURCES
  sudo apt update
  sudo apt install mtop
)
```

Run `/usr/bin/mtop` to launch the packaged copy if you previously installed
the manual launcher under `~/.local/bin`. The package keeps settings under
`$XDG_CONFIG_HOME/mtop` or `~/.config/mtop`. UPS readings require a working NUT
server with the UPS name described in the [README](../README.md).

To remove the package and its repository, run the following commands.

```sh
sudo apt remove mtop
sudo rm /etc/apt/sources.list.d/mtop.sources /etc/apt/keyrings/mtop-archive-keyring.asc
sudo apt update
```

## Build and publish

On Debian 12, install `build-essential`, `debhelper` and `cmake`, then run
`dpkg-buildpackage -us -uc -j2` from a clean checkout. The source and binary
packages appear in the parent directory. CI provides both architectures as
artifacts and tests installation on all three distributions.

For a package update, add a higher version to `debian/changelog` and merge the
change to `main`. Run **Debian packages and APT** from `main` with **publish**
selected. The workflow builds and tests both packages, signs the APT indexes,
tests installation through the signed archive, and deploys to GitHub Pages.
Each deployment carries the current package version for both architectures.

GitHub Pages uses GitHub Actions as its source. The `github-pages` environment
is restricted to `main` and holds the armored signing subkey export in
`APT_SIGNING_SUBKEY`. To provision a replacement subkey, use the offline
primary key to create a signing subkey, update
`docs/mtop-archive-keyring.asc` with the public export, and set the environment
secret from `gpg --armor --export-secret-subkeys PRIMARY_FINGERPRINT`. Publish
with the updated key and direct existing users to refresh their keyring before
the prior signing subkey expires. Replacing the primary key also requires
updating the published fingerprint and communicating it to users through a
trusted channel.

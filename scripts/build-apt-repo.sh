#!/usr/bin/env bash
set -euo pipefail

packages=$1
site=$2
repo="$site/apt"
release=dists/stable
version=$(dpkg-parsechangelog -S Version)
public_key=docs/mtop-archive-keyring.asc

mkdir -p "$repo/pool/main/m/mtop"
for arch in amd64 arm64; do
    deb="$packages/debian-$arch/mtop_${version}_$arch.deb"
    test "$(dpkg-deb -f "$deb" Package)" = mtop
    test "$(dpkg-deb -f "$deb" Architecture)" = "$arch"
    test "$(dpkg-deb -f "$deb" Version)" = "$version"
    install -m 644 "$deb" "$repo/pool/main/m/mtop/"
done

install -m 644 "$public_key" "$repo/mtop-archive-keyring.asc"
cat > "$site/index.html" <<'HTML'
<!doctype html>
<html lang="en">
<meta charset="utf-8">
<title>mtop APT repository</title>
<h1>mtop APT repository</h1>
<p><a href="https://github.com/Mattjhagen/mtop/blob/main/docs/apt.md">Installation instructions</a></p>
</html>
HTML

export GNUPGHOME
GNUPGHOME=$(mktemp -d)
trap 'gpgconf --kill gpg-agent; rm -rf "$GNUPGHOME"' EXIT
gpg --batch --import "$public_key"
fingerprint=$(gpg --batch --with-colons --list-keys | awk -F: '$1 == "fpr" && !found { print $10; found=1 }')
printf '%s\n' "${APT_SIGNING_SUBKEY:?Set APT_SIGNING_SUBKEY to the armored signing subkey export}" | gpg --batch --import
unset APT_SIGNING_SUBKEY

cd "$repo"
for arch in amd64 arm64; do
    index="$release/main/binary-$arch/Packages"
    mkdir -p "$(dirname "$index")"
    apt-ftparchive -a "$arch" packages pool > "$index"
    grep -qx "Architecture: $arch" "$index"
    gzip -n -k "$index"
done

apt-ftparchive \
    -o APT::FTPArchive::Release::Origin=mtop \
    -o APT::FTPArchive::Release::Label=mtop \
    -o APT::FTPArchive::Release::Suite=stable \
    -o APT::FTPArchive::Release::Codename=stable \
    -o 'APT::FTPArchive::Release::Architectures=amd64 arm64' \
    -o APT::FTPArchive::Release::Components=main \
    release "$release" > "$release/Release"

gpg --batch --local-user "$fingerprint" --clear-sign --output "$release/InRelease" "$release/Release"
gpg --batch --local-user "$fingerprint" --armor --detach-sign --output "$release/Release.gpg" "$release/Release"
gpg --batch --verify "$release/InRelease"

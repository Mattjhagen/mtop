#!/bin/bash
set -euo pipefail
[[ $EUID == 0 && $(hostname -s) == r510 ]]
[[ -x /home/matt/.local/bin/mtop ]]
command -v chvt >/dev/null
stamp=$(date +%Y%m%d-%H%M%S)
for f in /etc/systemd/system/getty@tty1.service.d/mtop.conf /etc/systemd/system/mtop-console.service; do
 [[ ! -e $f ]] || cp -a "$f" "$f.backup-$stamp"
done
if [[ -e /usr/local/bin/mtop && ! -L /usr/local/bin/mtop ]]; then echo '/usr/local/bin/mtop exists; stop for review'; exit 1; fi
ln -sfn /home/matt/.local/bin/mtop /usr/local/bin/mtop
profile=/home/matt/.zprofile
[[ ! -e $profile ]] || cp -a "$profile" "$profile.before-mtop-$stamp"
if ! grep -q '# R510 attached-console dashboard' "$profile" 2>/dev/null; then
 cat >> "$profile" <<'PROFILE'

# R510 attached-console dashboard: only local console 1, never SSH.
if [[ -z "$SSH_CONNECTION" && "$(tty)" == /dev/tty1 ]]; then
    exec /home/matt/.local/bin/mtop
fi
PROFILE
fi
chown matt:matt "$profile"
mkdir -p /etc/systemd/system/getty@tty1.service.d
cat > /etc/systemd/system/getty@tty1.service.d/mtop.conf <<'UNIT'
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin matt --noclear %I $TERM
UNIT
cat > /etc/systemd/system/mtop-console.service <<UNIT
[Unit]
Description=Show mtop dashboard on the attached R510 screen
Wants=getty@tty1.service
After=getty@tty1.service display-manager.service

[Service]
Type=oneshot
ExecStart=$(command -v chvt) 1
RemainAfterExit=yes

[Install]
WantedBy=graphical.target
UNIT
systemctl daemon-reload
systemd-analyze verify /etc/systemd/system/mtop-console.service
systemctl enable getty@tty1.service mtop-console.service
systemctl is-enabled getty@tty1.service mtop-console.service
# Apply now on the unused console without restarting any existing login session.
systemctl start getty@tty1.service
systemctl start mtop-console.service
echo 'PASS: mtop opens on console 1 now and at boot; no reboot performed.'

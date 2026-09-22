"""Update this user's existing Autodesk shortcuts; retain originals in logs."""
from pathlib import Path
import shutil
import os
import subprocess

project = Path(__file__).resolve().parent.parent
applications = Path.home() / ".local/share/applications"
inventor = applications / "wine/Programs/Autodesk Inventor 2027"
access = applications / "wine/Programs/Autodesk/Autodesk Access/Autodesk Access.desktop"
files = sorted(inventor.rglob("*.desktop"))
if access.exists():
    files.append(access)

def quote(value):
    return '"' + str(value).replace('\\', '\\\\\\\\').replace('"', '\\\\"').replace('`', '\\\\`').replace('$', '\\\\$') + '"'

for desktop in files:
    backup = project / "logs/kde-shortcut-backup" / desktop.relative_to(applications)
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        shutil.copy2(desktop, backup)
    args = [str(project / "run-kde-app.sh")]
    if desktop == access:
        args += ["--shortcut", "access"]
    elif desktop.stem != "Autodesk Inventor Professional 2027 - English":
        args += ["--shortcut", str(desktop.relative_to(inventor).with_suffix(".lnk"))]
    replacements = {
        "Exec": " ".join(map(quote, args)),
        "TryExec": str(project / "run-kde-app.sh"),
        "Path": str(project),
        "Comment": "Autodesk in the configured offline Wine research environment",
        "StartupNotify": "false",
        "Terminal": "false",
    }
    lines = []
    for line in desktop.read_text().splitlines():
        if line.partition("=")[0] not in replacements:
            lines.append(line)
    lines.extend(f"{key}={value}" for key, value in replacements.items())
    desktop.write_text("\n".join(lines) + "\n")
    print(desktop)

# Nix wrappers change XDG_DATA_DIRS, giving the shell a different cache from
# the running desktop. Refresh with Plasma's environment and the real binary.
cache_builder = shutil.which("kbuildsycoca6")
if cache_builder:
    binary = Path(cache_builder).resolve()
    unwrapped = binary.with_name(".kbuildsycoca6-wrapped")
    if unwrapped.exists():
        binary = unwrapped
    environment = os.environ.copy()
    for proc in Path("/proc").iterdir():
        if not proc.name.isdigit():
            continue
        try:
            if proc.stat().st_uid != os.getuid():
                continue
            if (proc / "comm").read_text().strip() != "plasmashell":
                continue
            environment = dict(item.decode().split("=", 1) for item in
                               (proc / "environ").read_bytes().split(b"\0") if b"=" in item)
            break
        except (OSError, UnicodeError):
            continue
    subprocess.run([str(binary), "--noincremental"], env=environment, check=True)

#!/usr/bin/env python3

import os
import subprocess
import urllib.request
import yaml

YAML_URL = "https://raw.githubusercontent.com/PS2Homebrew-arcade/pcsx2x6/master/bin/resources/GameIndex.yaml"

with urllib.request.urlopen(YAML_URL) as response:
    games = yaml.safe_load(response.read().decode("utf-8"))

for game_id, data in games.items():
    title = data["name"]
    boot = data["bootprog"]

    if boot == "???":
        continue

    bindir = f"bin/{game_id}"

    subprocess.run(
       [
           "make",
           "--silent",
           "clean",
           "all",
           "nkbu",
           f"BOOT_PATH={boot}",
           f"BINDIR={bindir}",
       ],
       check=True,
    )

    os.makedirs(bindir, exist_ok=True)

    with open(f"{bindir}/title.txt", "w", encoding="utf-8") as f:
        f.write(f"GameID: {game_id}\n")
        f.write(f"Title: {title}\n")
        f.write(f"BootProg: {boot}\n")

from Scons.Script import Import  # ACHTUNG: Scons groß/klein je nach PIO-Version; s. Hinweis unten
import os
import shutil

# ---- PlatformIO/SCons-Umgebung laden ----
Import("env")
project_dir = env["PROJECT_DIR"]

print(f"[clean-sdkconfig] Projektpfad: {project_dir}")

# 1) alle sdkconfig* entfernen (auch sdkconfig.esp32dev)
for name in os.listdir(project_dir):
    if name.startswith("sdkconfig"):
        path = os.path.join(project_dir, name)
        try:
            if os.path.isfile(path):
                os.remove(path)
                print(f"[clean-sdkconfig] Entfernt Datei: {path}")
            elif os.path.isdir(path):
                shutil.rmtree(path, ignore_errors=True)
                print(f"[clean-sdkconfig] Entfernt Verzeichnis: {path}")
        except Exception as e:
            print(f"[clean-sdkconfig] Konnte {path} nicht löschen: {e}")

# 2) (optional) alten Build-Ordner löschen
build_dir = os.path.join(project_dir, ".pio", "build")
if os.path.exists(build_dir):
    try:
        shutil.rmtree(build_dir, ignore_errors=True)
        print(f"[clean-sdkconfig] Build-Ordner gelöscht: {build_dir}")
    except Exception as e:
        print(f"[clean-sdkconfig] Konnte Build-Ordner nicht löschen: {e}")

# 3) sdkconfig.defaults sicherstellen/anpassen
defaults = os.path.join(project_dir, "sdkconfig.defaults")

# existierende Zeilen lesen (falls Datei schon da ist)
lines = []
if os.path.exists(defaults):
    with open(defaults, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

def set_line(key, value):
    line = f"{key}={value}\n"
    for i, l in enumerate(lines):
        if l.startswith(key + "="):
            lines[i] = line
            return
    lines.append(line)

# Pflicht für Arduino-as-IDF:
set_line("CONFIG_FREERTOS_HZ", "1000")

# Flash-Größe (Warnung vermeiden; für esp32dev üblich 4MB)
set_line("CONFIG_ESPTOOLPY_FLASHSIZE", "\"4MB\"")
if not any(l.startswith("CONFIG_ESPTOOLPY_FLASHSIZE_4MB=") for l in lines):
    lines.append("CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y\n")

with open(defaults, "w", encoding="utf-8") as f:
    f.writelines(lines)

print(f"[clean-sdkconfig] sdkconfig.defaults aktualisiert: {defaults}")

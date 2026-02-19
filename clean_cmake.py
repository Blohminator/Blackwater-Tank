import shutil, os

def before_build(source, target, env):
    # Pfad zum CMake-Ordner in .pio
    qpath = os.path.join(env['PROJECT_BUILD_DIR'], env['PIOENV'], '.cmake')
    if os.path.exists(qpath):
        print("⚠️ Lösche alten .cmake Ordner:", qpath)
        shutil.rmtree(qpath, ignore_errors=True)

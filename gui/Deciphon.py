# Compilation mode, support OS-specific options
# nuitka-project-if: {OS} in ("Darwin"):
#    nuitka-project: --macos-create-app-bundle
#    nuitka-project: --macos-target-arch=arm64
#    nuitka-project: --macos-app-icon={MAIN_DIRECTORY}/logo.icns
#    nuitka-project: --macos-app-name=Deciphon
#    nuitka-project: --macos-app-mode=gui
#    nuitka-project: --macos-app-version=1.0.0
#    nuitka-project: --include-module=ijson.backends.yajl2_c
#    nuitka-project: --include-module=ijson.backends._yajl2
#    nuitka-project: --output-dir=build
#    nuitka-project: --output-filename=deciphon
#    nuitka-project: --include-package-data=hmmer
#    nuitka-project: --include-package-data=deciphon_core
# nuitka-project-if: {OS} in ("Linux"):
#    nuitka-project: --standalone
#    nuitka-project: --linux-icon={MAIN_DIRECTORY}/logo.png
# nuitka-project: --product-name=Deciphon
# nuitka-project: --product-version=1.0.0
# nuitka-project: --output-dir=build
# nuitka-project: --include-package-data=hmmer
# nuitka-project: --include-package-data=deciphon_core
# nuitka-project: --include-package-data=deciphon_gui
from deciphon_gui.app import App

if __name__ == "__main__":
    App()

# Compilation mode, support OS-specific options
# nuitka-project-if: {OS} in ("Darwin"):
#    nuitka-project: --macos-app-icon={MAIN_DIRECTORY}/logo.icns
#    nuitka-project: --macos-app-name=Deciphon
#    nuitka-project: --macos-app-mode=gui
#    nuitka-project: --macos-app-version=1.0.0
#    nuitka-project: --output-filename=deciphon
#    nuitka-project: --include-package-data=hmmer
#    nuitka-project: --include-package-data=deciphon_core
# nuitka-project-if: {OS} in ("Linux"):
#    nuitka-project: --linux-icon={MAIN_DIRECTORY}/logo.png
# nuitka-project: --product-name=Deciphon
# nuitka-project: --product-version=1.0.0
# nuitka-project: --include-package-data=hmmer
# nuitka-project: --include-package-data=deciphon_core
# nuitka-project: --include-package-data=deciphon_gui
# nuitka-project: --enable-plugin=tk-inter
from deciphon_gui.app import App

if __name__ == "__main__":
    App()

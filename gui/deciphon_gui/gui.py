import json
import sys
from io import BytesIO

import customtkinter as ctk
from deciphon_schema import Gencode

from deciphon_gui.alignment import AlignmentFrame
from deciphon_gui.assets import theme_json
from deciphon_gui.banner import Banner
from deciphon_gui.library import LibraryFrame
from deciphon_gui.progress import ProgressFrame
from deciphon_gui.sequence import SequenceFrame


def load_theme():
    cls = ctk.ThemeManager
    cls.theme = json.load(BytesIO(theme_json()))

    # filter theme values for platform
    for key in cls.theme.keys():
        # check if values for key differ on platforms
        if "macOS" in cls.theme[key].keys():
            if sys.platform == "darwin":
                cls.theme[key] = cls.theme[key]["macOS"]
            elif sys.platform.startswith("win"):
                cls.theme[key] = cls.theme[key]["Windows"]
            else:
                cls.theme[key] = cls.theme[key]["Linux"]

    # fix name inconsistencies
    if "CTkCheckbox" in cls.theme.keys():
        cls.theme["CTkCheckBox"] = cls.theme.pop("CTkCheckbox")
    if "CTkRadiobutton" in cls.theme.keys():
        cls.theme["CTkRadioButton"] = cls.theme.pop("CTkRadiobutton")


class GUI(ctk.CTk):
    def __init__(self):
        super().__init__()
        ctk.set_appearance_mode("light")
        ctk.ThemeManager
        load_theme()
        self.geometry("1700x556+5250")
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(2, weight=1)
        self.title("Deciphon")
        self.configure(fg_color=ctk.ThemeManager.theme["CTkFrame"]["fg_color"])

        self.banner = Banner(self)
        self.banner.grid(row=0, column=0, columnspan=2, stick="nwe")
        self.banner.grid(pady=(0, 10))
        self.banner.grid(padx=(10, 0))

        self.library = LibraryFrame(self)
        self.library.grid(row=1, column=0, sticky="nw")
        self.library.grid(pady=(0, 0))
        self.library.grid(padx=(0, 0))
        self.library.set_gencode(Gencode.BAPP)

        self.sequence = SequenceFrame(self)
        self.sequence.grid(row=2, column=0, sticky="snew")
        self.sequence.grid(padx=(0, 0))

        self.alignment = AlignmentFrame(self)
        self.alignment.grid(row=1, column=1, rowspan=4, sticky="nsew")

        self.progress = ProgressFrame(self)
        self.progress.grid(row=3, column=0, sticky="nsew")
        self.progress.grid(pady=(0, 10))
        self.progress.grid(padx=(10, 10))

        self.update()
        self.minsize(800, self.winfo_height())

    def set_on_closing(self, callback):
        self.protocol("WM_DELETE_WINDOW", callback)

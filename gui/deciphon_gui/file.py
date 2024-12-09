from io import BytesIO
from pathlib import Path
from tkinter.filedialog import askopenfilename

import customtkinter as ctk
from deciphon_schema import HMMFile
from PIL import Image

from deciphon_gui.assets import file_disabled_png, file_enabled_png
from deciphon_gui.button import Button
from deciphon_gui.entry import Entry


class FileFrame(ctk.CTkFrame):
    def __init__(self, parent, title: str, label_width: int, *args, **kwargs):
        kwargs.setdefault("corner_radius", 0)
        super().__init__(parent, *args, **kwargs)
        self.configure(bg_color="transparent", fg_color="transparent")
        self.grid_columnconfigure(1, weight=1)

        self.label = ctk.CTkLabel(self, text=f"{title}:", width=label_width)
        self.label.configure(anchor="w")
        self.label.grid(row=0, column=0, sticky="nw")

        self.field = Entry(self, placeholder_text="Select an HMM file")
        self.field.configure(corner_radius=0)
        self.field.grid(row=0, column=1, sticky="nsew")

        enabled = BytesIO(file_enabled_png())
        disabled = BytesIO(file_disabled_png())
        image_enabled = ctk.CTkImage(
            light_image=Image.open(enabled),
            dark_image=Image.open(enabled),
            size=(20, 20),
        )
        image_disabled = ctk.CTkImage(
            light_image=Image.open(disabled),
            dark_image=Image.open(disabled),
            size=(20, 20),
        )

        button_size = 28
        self.button = Button(
            self, text="", image_enabled=image_enabled, image_disabled=image_disabled
        )
        self.button.configure(height=button_size, width=button_size)
        self.button.configure(corner_radius=0)
        self.button.configure(fg_color="#325881")
        self.button.grid(row=0, column=2, sticky="nsew")
        self.button.grid(padx=(0, 0))
        self.button.grid(pady=(0, 0))
        self.button.configure(command=self.on_select_file)

    def select(self, x: HMMFile):
        self.field.insert("0", str(x.path))

    def fetch(self) -> HMMFile:
        return HMMFile(path=Path(self.field.get()))

    def on_select_file(self):
        filename = askopenfilename(filetypes=[("Text files", ".hmm")])
        if len(filename) == 0:
            return

        self.field.delete("0", "end")
        self.field.insert("0", filename)

    def disable(self):
        self.field.disable()
        self.button.disable()

    def enable(self):
        self.field.enable()
        self.button.enable()

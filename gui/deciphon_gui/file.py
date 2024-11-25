from pathlib import Path
from PIL import Image
from tkinter.filedialog import askopenfilename

import customtkinter as ctk
from deciphon_core.schema import HMMFile

from deciphon_gui.entry import Entry
from deciphon_gui.button import Button


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

        image = ctk.CTkImage(
            light_image=Image.open(
                "/Users/horta/code/deciphon/gui/deciphon_gui/file_dark.png"
            ),
            dark_image=Image.open(
                "/Users/horta/code/deciphon/gui/deciphon_gui/file_light.png"
            ),
            size=(20, 20),
        )

        button_size = 28
        self.button = Button(self, text="", image=image)
        self.button.configure(height=button_size, width=button_size)
        self.button.configure(corner_radius=0)
        self.button.grid(row=0, column=2, sticky="nsew")
        self.button.grid(padx=(0, 0))
        self.button.grid(pady=(0, 0))
        self.button.configure(command=self.on_select_file)

        # self.home_button = customtkinter.CTkButton(self.navigation_frame, corner_radius=0, height=40, border_spacing=10, text="Home",
        #                                            fg_color="transparent", text_color=("gray10", "gray90"), hover_color=("gray70", "gray30"),
        #                                            image=self.home_image, anchor="w", command=self.home_button_event)

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

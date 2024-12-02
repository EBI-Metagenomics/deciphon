import customtkinter as ctk

from deciphon_gui.widget import disable, enable


class OptionMenu(ctk.CTkOptionMenu):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)

    def enable(self):
        enable(self)

    def disable(self):
        disable(self)

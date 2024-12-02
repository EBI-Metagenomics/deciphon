import customtkinter as ctk

from deciphon_gui.widget import disable, enable


class Entry(ctk.CTkEntry):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)

    def enable(self):
        enable(self)
        self.configure(text_color=ctk.ThemeManager.theme["CTkEntry"]["text_color"])

    def disable(self):
        disable(self)
        self.configure(
            text_color=ctk.ThemeManager.theme["CTkEntry"]["text_color_disabled"]
        )

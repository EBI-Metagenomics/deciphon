import customtkinter as ctk

from deciphon_gui.theme import disabled_fg_color
from deciphon_gui.widget import disable, enable


class Entry(ctk.CTkEntry):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self._fg_color_enabled = ctk.ThemeManager.theme["CTkEntry"]["fg_color"]
        self._fg_color_disabled = disabled_fg_color()

    def enable(self):
        enable(self)
        self.configure(fg_color=self._fg_color_enabled)

    def disable(self):
        disable(self)
        self.configure(fg_color=self._fg_color_disabled)

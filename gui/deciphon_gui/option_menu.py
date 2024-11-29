import customtkinter as ctk

from deciphon_gui.widget import disable, enable
from deciphon_gui.theme import disabled_fg_color


class OptionMenu(ctk.CTkOptionMenu):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self._fg_color_enabled = ctk.ThemeManager.theme["CTkOptionMenu"]["fg_color"]
        self._fg_color_disabled = disabled_fg_color()

    def enable(self):
        enable(self)
        self.configure(fg_color=self._fg_color_enabled)

    def disable(self):
        disable(self)
        self.configure(fg_color=self._fg_color_disabled)

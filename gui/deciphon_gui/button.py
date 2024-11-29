import customtkinter as ctk

from deciphon_gui.theme import disabled_fg_color
from deciphon_gui.widget import disable, enable


class Button(ctk.CTkButton):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        # self._disabled_fg_color = disabled_fg_color()
        # self._enabled_fg_color = ctk.ThemeManager.theme["CTkButton"]["fg_color"]

    def disable(self):
        # self.configure(fg_color=self._enabled_fg_color)
        disable(self)

    def enable(self):
        # self.configure(fg_color=self._disabled_fg_color)
        enable(self)

import customtkinter as ctk

from deciphon_gui.theme import font_family_mono
from deciphon_gui.widget import disable, enable, get_state


class TextArea(ctk.CTkTextbox):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.configure(font=ctk.CTkFont(family=font_family_mono()))
        self.configure(corner_radius=0)

    def set_lines(self, rows: list[str]):
        old_state = get_state(self)
        self.configure(state="normal")
        self.delete("0.0", "end")
        for i, x in enumerate(rows):
            self.insert(f"{i+1}.0", f"{x}\n")
        self.configure(state=old_state)

    def get_lines(self):
        return self.get("0.0", "end").split("\n")

    def enable(self):
        self.configure(text_color=ctk.ThemeManager.theme["CTkTextbox"]["text_color"])
        enable(self)

    def disable(self, change_color=True):
        if change_color:
            self.configure(
                text_color=ctk.ThemeManager.theme["CTkTextbox"]["text_color_disabled"]
            )
        disable(self)

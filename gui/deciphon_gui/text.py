import customtkinter as ctk

from deciphon_gui.theme import disabled_fg_color
from deciphon_gui.widget import get_state, disable, enable
from deciphon_gui.heading import h2


class TextAreaFrame(ctk.CTkFrame):
    def __init__(self, parent, title: str, family: str, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)

        self.title = h2(self, title)
        self.title.grid(row=0, column=0, sticky="w")
        self.title.grid(padx=(10, 10))
        self.title.grid(pady=(0, 0))

        self.box = ctk.CTkTextbox(self)
        self.box.grid(row=1, column=0, sticky="nsew")
        self.box.grid(padx=(10, 10))
        self.box.grid(pady=(0, 10))
        self.box.configure(font=ctk.CTkFont(family=family))
        self._disabled_fg_color = self["background"]
        self._enabled_fg_color = self.box["background"]

    def set_text(self, rows: list[str]):
        old_state = get_state(self.box)
        self.box.configure(state="normal")
        self.box.delete("0.0", "end")
        for i, x in enumerate(rows):
            self.box.insert(f"{i+1}.0", f"{x}\n")
        self.box.configure(state=old_state)

    def enable(self):
        self.box.configure(fg_color=self._enabled_fg_color)
        enable(self.box)

    def disable(self):
        self.box.configure(fg_color=self._disabled_fg_color)
        disable(self.box)


class TextArea(ctk.CTkTextbox):
    def __init__(self, parent, family: str, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.configure(font=ctk.CTkFont(family=family))
        self.configure(corner_radius=0)
        self._disabled_fg_color = disabled_fg_color()
        self._enabled_fg_color = ctk.ThemeManager.theme["CTkTextbox"]["fg_color"]

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
        self.configure(fg_color=self._enabled_fg_color)
        enable(self)

    def disable(self):
        self.configure(fg_color=self._disabled_fg_color)
        disable(self)
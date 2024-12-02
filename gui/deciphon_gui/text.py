import customtkinter as ctk

from deciphon_gui.heading import h2
from deciphon_gui.widget import disable, enable, get_state


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

    def set_text(self, rows: list[str]):
        old_state = get_state(self.box)
        self.box.configure(state="normal")
        self.box.delete("0.0", "end")
        for i, x in enumerate(rows):
            self.box.insert(f"{i+1}.0", f"{x}\n")
        self.box.configure(state=old_state)

    def enable(self):
        enable(self.box)

    def disable(self):
        disable(self.box)


class TextArea(ctk.CTkTextbox):
    def __init__(self, parent, family: str, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.configure(font=ctk.CTkFont(family=family))
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

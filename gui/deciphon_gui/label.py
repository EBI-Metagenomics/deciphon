import customtkinter as ctk

from deciphon_gui.entry import Entry
from deciphon_gui.heading import h3
from deciphon_gui.option_menu import OptionMenu


class LabelField(ctk.CTkFrame):
    def __init__(
        self,
        parent,
        title: str,
        placeholder_text: str | None = None,
        *args,
        **kwargs,
    ):
        kwargs["corner_radius"] = kwargs.get("corner_radius", 0)
        super().__init__(parent, *args, **kwargs)
        self.grid_columnconfigure(1, weight=1)
        self.configure(bg_color="transparent", fg_color="transparent")

        self.label = h3(self, f"{title}:")
        self.label.configure(anchor="w")
        self.label.grid(row=0, column=0, sticky="nw")
        self.label.grid(padx=(0, 0))
        self.label.grid(pady=(0, 0))

        self.entry = Entry(self, placeholder_text=placeholder_text)
        self.entry.configure(corner_radius=0)
        self.entry.grid(row=0, column=1, sticky="new")
        self.entry.grid(pady=(0, 0))

    def set_text(self, x: str):
        self.entry.insert("0", x)

    def get_text(self):
        return self.entry.get()

    def disable(self):
        self.entry.disable()

    def enable(self):
        self.entry.enable()


class LabelMenu(ctk.CTkFrame):
    def __init__(
        self,
        parent,
        title: str,
        values: list[str],
        *args,
        **kwargs,
    ):
        kwargs.setdefault("corner_radius", 0)
        super().__init__(parent, *args, **kwargs)
        self.configure(bg_color="transparent", fg_color="transparent")

        self.label = ctk.CTkLabel(self, text=f"{title}:")
        self.label.configure(anchor="w")
        self.label.grid(row=0, column=0, sticky="nw")
        self.label.grid(padx=(0, 0))
        self.label.grid(pady=(0, 0))

        self.menu = OptionMenu(self, values=values)
        self.menu.grid(row=0, column=1, sticky="new")
        self.menu.grid(padx=(0, 0))
        self.menu.grid(pady=(0, 0))
        self.menu.configure(corner_radius=0)

    def set(self, x: str):
        self.menu.set(x)

    def get(self):
        return self.menu.get()

    def disable(self):
        self.menu.disable()

    def enable(self):
        self.menu.enable()

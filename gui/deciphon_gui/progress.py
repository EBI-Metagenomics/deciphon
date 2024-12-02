import customtkinter as ctk
from deciphon_gui.heading import h2, h3
from deciphon_gui.shorten import shorten


class ProgressFrame(ctk.CTkFrame):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.grid_columnconfigure(0, weight=1)

        self.label = h2(self, "Progress")
        self.label.configure(anchor="w")
        self.label.grid(row=0, column=0, sticky="nw")
        self.label.grid(padx=(0, 0))
        self.label.grid(pady=(0, 0))

        self.bar = ctk.CTkProgressBar(self)
        self.bar.configure(corner_radius=0)
        self.bar.grid(row=1, column=0, sticky="snew")
        self.bar.grid(padx=(0, 0))
        self.bar.set(0)

        self.status = h3(self, "Idle...")
        self.status.configure(anchor="w")
        self.status.grid(row=2, column=0, sticky="snew")
        self._default_text_color = ctk.ThemeManager.theme["CTkLabel"]["text_color"]
        self.info("idle.")

    def set_progress(self, x: float):
        self.bar.set(x)

    def info(self, x: str):
        msg = shorten(x, 98)
        self.status.configure(text=f"Status: {msg}")
        self.status.configure(text_color=self._default_text_color)

    def error(self, x: str):
        msg = shorten(x, 98)
        self.status.configure(text=f"Status: {msg}")
        self.status.configure(text_color="#D32F2E")

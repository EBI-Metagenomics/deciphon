from io import StringIO

import customtkinter as ctk
from deciphon_core.sequence import Sequence
from fasta_reader import Reader

from deciphon_gui.heading import h2
from deciphon_gui.submit import SubmitFrame
from deciphon_gui.text import TextArea


class SequenceFrame(ctk.CTkFrame):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self.configure(corner_radius=0)

        self.title = h2(self, "Sequence")
        self.title.grid(row=0, column=0, sticky="nw")
        self.title.grid(padx=(10, 10))
        self.title.grid(pady=(0, 0))

        self.text = TextArea(self)
        self.text.grid(row=1, column=0, sticky="nsew")
        self.text.grid(padx=(10, 10))
        self.text.grid(pady=(0, 10))

        self.submit = SubmitFrame(self, "Cancel", "Search")
        self.submit.grid(row=2, column=0, sticky="nsew")
        self.submit.grid(padx=(0, 0))
        self.submit.grid(pady=(0, 10))

    def disable(self):
        self.disable_text()
        self.submit.disable()

    def enable_text(self):
        self.text.enable()

    def disable_text(self):
        self.text.disable()

    def enable_cancel(self):
        self.submit.enable_cancel()

    def enable_search(self):
        self.submit.enable_submit()

    def disable_cancel(self):
        self.submit.disable_cancel()

    def disable_submit(self):
        self.submit.disable_submit()

    def set_lines(self, lines: list[str]):
        self.text.set_lines(lines)

    def get_sequences(self):
        reader = Reader(StringIO("\n".join(self.text.get_lines())))
        return [Sequence(i, x.defline, x.sequence) for i, x in enumerate(reader)]

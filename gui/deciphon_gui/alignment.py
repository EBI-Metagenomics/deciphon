import customtkinter as ctk
from deciphon_core.schema import SnapFile

from deciphon_gui.heading import h2
from deciphon_gui.text import TextArea
from deciphon_snap.read_snap import read_snap
from deciphon_snap.view import view_alignments


class AlignmentFrame(ctk.CTkFrame):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self.configure(corner_radius=0)

        self.title = h2(self, "Alignment")
        self.title.grid(row=0, column=0, sticky="nw")
        self.title.grid(padx=(10, 10))
        self.title.grid(pady=(0, 0))

        self.text = TextArea(self, "Roboto Mono")
        self.text.grid(row=1, column=0, sticky="nsew")
        self.text.grid(padx=(10, 10))
        self.text.grid(pady=(0, 10))
        self.text.disable()
        self.set_empty()

    def set_alignment(self, snap: SnapFile):
        view = view_alignments(read_snap(snap.path))
        lines = [x for rows in view for x in rows.split("\n")]
        indices = [i for i, x in enumerate(reversed(lines)) if len(x) > 0]
        if len(indices) > 0:
            lines = lines[: len(lines) - indices[0]]
        if len(lines) == 0:
            self.text.set_lines(["No annotation has been found."])
        else:
            self.text.set_lines(lines)

    def set_empty(self):
        self.text.set_lines(["No alignment yet..."])

from collections.abc import Callable
import customtkinter as ctk

from deciphon_gui.file import FileFrame
from deciphon_gui.gencode import Gencode, GencodeName, gencode_description
from deciphon_gui.heading import h2
from deciphon_gui.label import LabelField, LabelMenu
from deciphon_gui.submit import SubmitFrame
from deciphon_gui.epsilon import Epsilon

GENCODES = {i.value: gencode_description(i, j) for i, j in zip(Gencode, GencodeName)}


class LibraryFrame(ctk.CTkFrame):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.grid_columnconfigure(1, weight=1)
        self.configure(corner_radius=0)

        self.title = h2(self, "Library")
        self.title.grid(row=0, column=0, columnspan=2, sticky="nw")
        self.title.grid(padx=(10, 0))
        self.title.grid(pady=(0, 0))

        LEFT_LABEL_WIDTH = 88
        self.gencode = LabelMenu(self, "Genetic code", list(GENCODES.values()))
        self.gencode.label.configure(width=LEFT_LABEL_WIDTH)
        self.gencode.menu.configure(width=365)
        self.gencode.grid(row=1, column=0)
        self.gencode.grid(padx=(10, 10))
        self.gencode.grid(pady=(0, 4))

        self.error = LabelField(self, "Error probability", "e.g., 0.01")
        self.error.label.configure(width=105)
        self.error.entry.configure(width=80)
        self.error.grid(row=1, column=1)
        self.error.grid(padx=(0, 10))
        self.error.grid(pady=(0, 4))
        self.error.grid(sticky="we")

        self.hmm = FileFrame(self, "HMM file", LEFT_LABEL_WIDTH)
        self.hmm.grid(row=2, column=0, columnspan=2)
        self.hmm.grid(sticky="we")
        self.hmm.grid(padx=(10, 10))
        self.hmm.grid(pady=(0, 10))

        self.submit = SubmitFrame(self, "Cancel", "Load")
        self.submit.grid(row=3, column=0, columnspan=2)
        self.submit.grid(padx=(0, 0))
        self.submit.grid(pady=(0, 10))
        self.submit.grid(sticky="we")

    def set_gencode(self, gencode: Gencode):
        self.gencode.set(GENCODES[gencode])

    def get_gencode(self):
        x = [k for k, v in GENCODES.items() if v == self.gencode.get()][0]
        return Gencode(x)

    def get_error(self):
        return Epsilon.model_validate({"value": float(self.error.entry.get())})

    def use_load(self, x: bool):
        if x:
            self.submit.submit.configure(text="Load")
        else:
            self.submit.submit.configure(text="Unload")

    def is_load(self):
        return self.submit.submit.cget("text") == "Load"

    def enable_cancel(self):
        self.submit.enable_cancel()

    def enable_submit(self):
        self.submit.enable_submit()

    def disable_cancel(self):
        self.submit.disable_cancel()

    def disable_submit(self):
        self.submit.disable_submit()

    def on_enter_callback(self, callback: Callable[[], None]):
        def ignore_argument(event):
            return callback()

        self.hmm.field.bind("<Return>", ignore_argument)
        self.error.entry.bind("<Return>", ignore_argument)

    def disable_input(self):
        self.gencode.disable()
        self.error.disable()
        self.hmm.disable()

    def enable_input(self):
        self.gencode.enable()
        self.error.enable()
        self.hmm.enable()

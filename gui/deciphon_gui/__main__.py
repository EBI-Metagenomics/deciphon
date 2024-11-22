from enum import Enum, auto
import tkinter
import tkinter.messagebox
from io import StringIO
from pathlib import Path
from subprocess import DEVNULL
from tkinter import IntVar

import customtkinter
from deciphon_worker.future import FutureSuccess
import psutil
from customtkinter import CTkButton, CTkEntry, CTkTextbox
from deciphon.h3daemon import H3Daemon
from deciphon.read_sequences import read_sequences
from deciphon_core.batch import Batch
from deciphon_core.scan import Scan
from deciphon_core.schema import HMMFile, NewSnapFile, SnapFile
from deciphon_core.sequence import Sequence
from deciphon_snap.read_snap import read_snap
from deciphon_snap.view import view_alignments
from deciphon_worker.scanner import Scanner, ScannerBoot, ScannerConfig, ScannerProduct
from fasta_reader.reader import Reader
from more_itertools import mark_ends
from tkinter.filedialog import askopenfilename

customtkinter.set_appearance_mode("System")
# Themes: "blue" (standard), "green", "dark-blue"
customtkinter.set_default_color_theme("blue")
# customtkinter.set_default_color_theme("green")


def lift_window(window):
    window.attributes("-topmost", True)
    window.update_idletasks()  # get window on top
    window.attributes("-topmost", False)  # prevent permanent focus
    window.focus_force()  # focus to the window


LONGPAD = 20
SEMIPAD = 10
SHRTPAD = 0

TXTCOLOR = ("gray10", "#DCE4EE")


def enable(x: CTkButton):
    x.configure(state="normal")


def disable(x: CTkButton):
    x.configure(state="disabled")


class State(Enum):
    idle = auto()
    loading = auto()
    pressing = auto()
    scanning = auto()


class App(customtkinter.CTk):
    def __init__(self):
        super().__init__()
        self.state = State.idle
        self._boot: ScannerBoot | None = None
        self._boot_starting: FutureSuccess | None = None
        self._scanner: Scanner | None = None
        self._product: ScannerProduct | None = None
        self.library_file: Path | None = None
        self._setup_widgets()
        lift_window(self)
        disable(self.cancel)
        enable(self.search)

    def _setup_widgets(self):
        self.title("CustomTkinter complex_example.py")
        self.geometry("1100x800")
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(3, weight=1)

        consensus = open("/Users/horta/code/deciphon/gui/consensus.fna", "r").read()
        self.sequence = CTkTextbox(self, width=250)
        self.sequence.grid(row=0, column=0, columnspan=5, sticky="nsew")
        self.sequence.grid(padx=(LONGPAD, LONGPAD), pady=(LONGPAD, SHRTPAD))
        self.sequence.insert("0.0", consensus)

        self.choose = CTkButton(self, text="Choose Library", text_color=TXTCOLOR)
        self.choose.configure(fg_color="transparent", border_width=2)
        self.choose.configure(command=self.choose_callback)
        self.choose.grid(row=1, column=1, sticky="nsew")
        self.choose.grid(padx=(SHRTPAD, SEMIPAD), pady=(LONGPAD, LONGPAD))

        self.choose_label = customtkinter.CTkLabel(self, text="No library chosen")
        self.choose_label.configure(anchor="w", width=150, fg_color="transparent")
        self.choose_label.grid(row=1, column=2, sticky="nsew")
        self.choose_label.grid(padx=(SHRTPAD, LONGPAD), pady=(LONGPAD, LONGPAD))

        self.cancel = CTkButton(self, text="Cancel", text_color=TXTCOLOR)
        self.cancel.configure(fg_color="transparent", border_width=2)
        self.cancel.configure(command=self.cancel_callback)
        self.cancel.grid(row=1, column=3, sticky="nsew")
        self.cancel.grid(padx=(SHRTPAD, LONGPAD), pady=(LONGPAD, LONGPAD))

        self.search = CTkButton(self, text="Search", text_color=TXTCOLOR)
        self.search.configure(fg_color="transparent", border_width=2)
        self.search.configure(command=self.search_callback)
        self.search.grid(row=1, column=4, sticky="nsew")
        self.search.grid(padx=(SHRTPAD, LONGPAD), pady=(LONGPAD, LONGPAD))

        self.progressbar = customtkinter.CTkProgressBar(master=self)
        self.progressbar.grid(row=2, column=0, columnspan=5, sticky="nsew")
        self.progressbar.grid(padx=(LONGPAD, LONGPAD), pady=(SHRTPAD, LONGPAD))
        self.progressbar.set(0)

        self.alignment = CTkTextbox(self, width=250, state="disabled")
        self.alignment.grid(row=3, column=0, columnspan=5, sticky="nsew")
        self.alignment.grid(padx=(LONGPAD, LONGPAD), pady=(SHRTPAD, LONGPAD))

    def _monitor(self):
        if self.state == State.loading:
            assert self._boot is not None
            assert self._boot_starting is not None
            if not self._boot_starting.done:
                self.after(100, self._monitor)
            else:
                print("Finished")
        elif self._product is not None:
            self.progressbar.set(self._product.progress / 100)
            if self._product.done:
                try:
                    self._product.result()
                    self.set_alignment(read_snap(self._product.result().path))
                finally:
                    self._product = None
                    disable(self.cancel)
                    enable(self.search)
            else:
                self.after(100, self._monitor)

    def choose_callback(self):
        filename = askopenfilename(filetypes=[("Text files", ".hmm")])
        self.library_file = Path(filename)
        self.choose_label.configure(text=self.library_file.name)
        self.start_loading()

    def start_loading(self):
        self.state = State.loading
        assert self.library_file is not None
        hmm = HMMFile(path=self.library_file)
        self._boot = ScannerBoot(ScannerConfig(hmm, 1, True, False, None, None, True))
        self._boot_starting = self._boot.start()
        self.after(100, self._monitor)

    def cancel_callback(self):
        if self._product is not None:
            self._product.interrupt()

    def search_callback(self):
        reader = Reader(StringIO(self.sequence.get("0.0", "end")))
        seqs = [Sequence(i, x.defline, x.sequence) for i, x in enumerate(reader)]
        if len(seqs) == 0:
            return

        snap = NewSnapFile(path=Path("snap.dcs"))

        if self._scanner is not None:
            self._scanner.stop().wait()
            self._scanner = None

        if self._boot is not None:
            self._boot.stop().wait()
            self._boot = None

        # hmmfile = HMMFile(path=Path(self.library.get()))
        hmmfile = HMMFile(path=Path("minifam.hmm"))
        self._boot = ScannerBoot(
            ScannerConfig(hmmfile, 1, True, False, None, None, False)
        )
        self._boot.start().wait()
        self._scanner = Scanner(self._boot.scan)
        self._scanner.start().wait()

        self._product = self._scanner.put(snap, seqs)
        enable(self.cancel)
        disable(self.search)
        self.after(100, self._monitor)

    def set_alignment(self, snap):
        self.alignment.configure(state="normal")
        self.alignment.delete("0.0", "end")
        for i, x in enumerate(mark_ends(iter(view_alignments(snap)))):
            if x[1]:
                self.alignment.insert(f"{i}.0", x[2].rstrip("\n"))
            else:
                self.alignment.insert(f"{i}.0", x[2])
        self.alignment.configure(state="disabled")

    def cleanup(self):
        if self._scanner is not None:
            self._scanner.stop().wait()
            self._scanner = None

        if self._boot is not None:
            self._boot.stop().wait()
            self._boot = None


if __name__ == "__main__":
    app = App()
    app.eval("tk::PlaceWindow . center")
    app.mainloop()
    app.cleanup()

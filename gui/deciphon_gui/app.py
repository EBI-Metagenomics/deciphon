from concurrent.futures import CancelledError, Future
from contextlib import suppress
from pathlib import Path
from subprocess import DEVNULL

from deciphon_core.schema import DBFile, HMMFile, NewSnapFile, SnapFile
from deciphon_worker import Progressor, Scanner, launch_scanner, press
from deciphon_worker.interrupted import Interrupted
from fasta_reader.errors import ParsingError
from pydantic import ValidationError

from deciphon_gui.example import example_hmmfile, example_sequence
from deciphon_gui.gui import GUI


class App:
    def __init__(self):
        self.gui = GUI()
        self._db: DBFile | None = None
        self._press_future: Progressor[DBFile] | None = None
        self._scanner_startup: Future[Scanner] | None = None
        self._scanner: Scanner | None = None
        self._scan_future: Progressor[SnapFile] | None = None
        self._scanner_shutdown: Future[None] | None = None
        self.gui.library.error.set_text("0.01")
        hmmfile = HMMFile(path=example_hmmfile())
        self.gui.library.hmm.select(hmmfile)
        self.gui.library.submit.on_cancel(self.library_on_cancel)
        self.gui.library.submit.on_submit(self.library_on_submit)
        self.gui.sequence.submit.on_cancel(self.sequence_on_cancel)
        self.gui.sequence.submit.on_submit(self.sequence_on_search)
        self.gui.library.disable_cancel()
        self.gui.library.enable_submit()
        self.gui.sequence.disable()
        self.gui.sequence.set_lines(example_sequence())
        self.gui.library.on_enter_callback(self.library_on_enter)
        self.gui.set_on_closing(self.on_closing)
        self.gui.mainloop()

    def library_on_enter(self):
        self.load_library()

    def library_on_cancel(self):
        print("library_on_cancel")
        assert self._press_future is not None
        self.gui.library.disable_cancel()
        if not self._press_future.cancel():
            self._press_future.interrupt()

    def library_on_submit(self):
        if self.gui.library.is_load():
            self.load_library()
        else:
            self.unload_library()

    def load_library(self):
        gencode = self.gui.library.get_gencode()
        try:
            error = self.gui.library.get_error()
        except (ValueError, ValidationError):
            self.gui.progress.error(
                "invalid error probability (must be a real number between zero and one)."
            )
            return
        try:
            hmmfile = self.gui.library.hmm.fetch()
        except ValidationError as exception:
            error = exception.errors()[0]
            if error["type"] == "path_not_file":
                self.gui.progress.error("not a file path.")
            else:
                self.gui.progress.error(f"invalid file path ({exception})")
            return
        if hmmfile.path.with_suffix(".dcp").exists():
            hmmfile.path.with_suffix(".dcp").unlink()
        self._press_future = press(hmmfile, gencode, error.value)
        self.gui.library.disable_input()
        self.gui.library.enable_cancel()
        self.gui.library.disable_submit()
        self.gui.sequence.enable_text()
        self.gui.sequence.disable_cancel()
        self.gui.sequence.disable_submit()
        self.poll_now()

    def unload_library(self):
        assert self._scanner is not None
        self._scanner_shutdown = self._scanner.shutdown()
        self._scanner = None
        self.gui.library.disable_submit()
        self.gui.sequence.disable_submit()
        self.poll_now()

    def sequence_on_cancel(self):
        print("sequence_on_cancel")
        assert self._scan_future is not None
        self.gui.sequence.disable_cancel()
        if not self._scan_future.cancel():
            self._scan_future.interrupt()

    def sequence_on_search(self):
        print("sequence_on_search")
        try:
            seqs = self.gui.sequence.get_sequences()
        except ParsingError as exception:
            self.gui.progress.error(f"invalid sequence. Parsing error ({exception}).")
            return
        if len(seqs) == 0:
            self.gui.progress.error("you must provide at least one sequence.")
            return
        snap = NewSnapFile(path=Path("snap.dcs"))
        self._scan_future = self.scanner.put(snap, seqs)
        self.gui.sequence.enable_cancel()
        self.gui.sequence.disable_submit()
        self.gui.sequence.disable_text()
        self.gui.alignment.set_empty()
        self.poll_now()

    def poll(self):
        print("poll")

        if self._press_future:
            if self._press_future.done():
                try:
                    self._db = self._press_future.result()
                    self.gui.progress.set_progress(1.0)
                    self._scanner_startup = launch_scanner(
                        self._db, stdout=DEVNULL, stderr=DEVNULL
                    )
                except (Interrupted, CancelledError):
                    self.gui.progress.info("cancelled.")
                except Exception as exception:
                    self.gui.progress.error(f"failed to press library ({exception}).")
                finally:
                    self._press_future = None
                    self.gui.library.use_load(False)
                    self.gui.library.disable_cancel()
                    self.gui.library.enable_submit()
            else:
                self.gui.progress.set_progress(self._press_future.progress / 100)
                self.gui.progress.info("pressing...")

        elif self._scanner_startup:
            if self._scanner_startup.done():
                try:
                    self._scanner = self._scanner_startup.result()
                    self.gui.progress.info("library is ready to use!")
                    self.gui.sequence.disable_cancel()
                    self.gui.sequence.enable_search()
                    self.gui.sequence.text.enable()
                except (Interrupted, CancelledError):
                    self.gui.progress.info("cancelled.")
                    self.gui.library.enable_input()
                    self.gui.library.disable_cancel()
                    self.gui.library.enable_submit()
                    self.gui.library.use_load(True)
                    self.gui.sequence.disable_cancel()
                    self.gui.sequence.disable_submit()
                    self.gui.sequence.text.enable()
                except Exception as exception:
                    print(exception)
                    self.gui.progress.error(f"failed to load library ({exception}).")
                    self.gui.library.enable_input()
                    self.gui.library.disable_cancel()
                    self.gui.library.enable_submit()
                    self.gui.library.use_load(True)
                    self.gui.sequence.disable_cancel()
                    self.gui.sequence.disable_submit()
                finally:
                    self._scanner_startup = None
            else:
                self.gui.progress.info("loading...")

        elif self._scan_future:
            print("Ponto 1")
            if self._scan_future.done():
                print("Ponto 2")
                try:
                    snap = self._scan_future.result()
                    self.gui.progress.set_progress(1.0)
                    self.gui.progress.info("annotation has finished!")
                    self.gui.alignment.set_alignment(snap)
                except (Interrupted, CancelledError):
                    self.gui.progress.info("cancelled.")
                except Exception as exception:
                    self.gui.progress.error(
                        f"failed to annotate sequence ({exception})."
                    )
                finally:
                    self._scan_future = None
                    self.gui.sequence.disable_cancel()
                    self.gui.sequence.enable_text()
                    self.gui.sequence.enable_search()

            else:
                print("Ponto 3")
                self.gui.progress.set_progress(self._scan_future.progress / 100)
                self.gui.progress.info("annotating...")

        elif self._scanner_shutdown:
            if self._scanner_shutdown.done():
                try:
                    self._scanner_shutdown.result()
                except Exception as exception:
                    print(exception)
                finally:
                    self.gui.progress.info("unloaded.")
                    self.gui.library.enable_submit()
                    self.gui.library.use_load(True)
                    self.gui.library.enable_input()
                    self.gui.sequence.disable()
                    self._scanner_shutdown = None
            else:
                self.gui.progress.info("shutting down...")

        if (
            self._press_future
            or self._scanner_startup
            or self._scan_future
            or self._scanner_shutdown
        ):
            self.poll_later()

    def poll_now(self):
        self.gui.after(0, self.poll)

    def poll_later(self):
        self.gui.after(100, self.poll)

    @property
    def scanner(self):
        assert self._scanner is not None
        return self._scanner

    @property
    def scan_future(self):
        assert self._scan_future is not None
        return self._scan_future

    def on_closing(self):
        if self._scan_future is not None:
            with suppress(Exception):
                if not self._scan_future.cancel():
                    self._scan_future.interrupt()

        if self._scanner is not None:
            with suppress(Exception):
                self._scanner.shutdown()

        if self._press_future is not None:
            with suppress(Exception):
                if not self._press_future.cancel():
                    self._press_future.interrupt()

        self.gui.destroy()

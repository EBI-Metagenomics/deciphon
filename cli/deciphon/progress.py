from time import sleep
import rich.progress
from threading import Event, Thread
from deciphon_core.scan import Scan


class Progress:
    def __init__(self, scan: Scan, disabled=False):
        self._continue = Event()
        self._scan = scan
        self._thread = Thread(target=self.progress_entry)
        self._disabled = disabled

    def start(self):
        if not self._disabled:
            self._thread.start()

    def progress_entry(self):
        with rich.progress.Progress() as progress:
            task = progress.add_task("Scanning", total=100)
            while not self._continue.is_set():
                progress.update(task, completed=self._scan.progress())
                sleep(0.35)
            progress.update(task, completed=self._scan.progress())

    def stop(self):
        if not self._disabled:
            self._continue.set()
            self._thread.join()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()

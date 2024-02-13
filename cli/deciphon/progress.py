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
        scan = self._scan
        last_progress = scan.progress()
        with rich.progress.Progress() as progress:
            task = progress.add_task("Scanning", total=100)
            console = progress.console
            while not self._continue.is_set():
                new_progress = scan.progress()
                progress.update(task, completed=new_progress)
                if not console.is_interactive and last_progress < new_progress:
                    progress.print(progress)
                sleep(0.35)
                last_progress = new_progress
            progress.update(task, completed=scan.progress())

    def stop(self):
        if not self._disabled:
            self._continue.set()
            self._thread.join()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()

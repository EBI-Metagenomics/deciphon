from collections.abc import Callable

import customtkinter as ctk

from deciphon_gui.widget import disable, enable


class SubmitFrame(ctk.CTkFrame):
    def __init__(
        self,
        parent,
        cancel_label="Cancel",
        submit_label="Submit",
        width=140,
        *args,
        **kwargs,
    ):
        super().__init__(parent, *args, **kwargs)
        self.configure(bg_color="transparent", fg_color="transparent")
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(3, weight=1)

        self.cancel = ctk.CTkButton(self, text=cancel_label, width=width)
        self.cancel.grid(row=0, column=1)
        self.cancel.grid(padx=(0, 9), pady=(0, 0))
        self.cancel.configure(corner_radius=0)

        self.submit = ctk.CTkButton(self, text=submit_label, width=width)
        self.submit.grid(row=0, column=2)
        self.submit.configure(corner_radius=0)

    def on_cancel(self, callback: Callable[[], None]):
        self.cancel.configure(command=callback)

    def on_submit(self, callback: Callable[[], None]):
        self.submit.configure(command=callback)

    def disable(self):
        self.disable_cancel()
        self.disable_submit()

    def enable_cancel(self):
        enable(self.cancel)

    def enable_submit(self):
        enable(self.submit)

    def disable_cancel(self):
        disable(self.cancel)

    def disable_submit(self):
        disable(self.submit)

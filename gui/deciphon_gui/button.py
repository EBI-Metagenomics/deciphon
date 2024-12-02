import customtkinter as ctk

from deciphon_gui.widget import disable, enable


class Button(ctk.CTkButton):
    def __init__(self, parent, *args, **kwargs):
        self._image_enabled = kwargs.pop("image_enabled", None)
        self._image_disabled = kwargs.pop("image_disabled", None)
        super().__init__(parent, *args, **kwargs)
        self.enable()

    def disable(self):
        if self._image_disabled is not None:
            self.configure(image=self._image_disabled)
        disable(self)

    def enable(self):
        if self._image_enabled is not None:
            self.configure(image=self._image_enabled)
        enable(self)

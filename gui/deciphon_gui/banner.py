import customtkinter as ctk


class Banner(ctk.CTkFrame):
    def __init__(
        self,
        parent,
        *args,
        **kwargs,
    ):
        kwargs.setdefault("corner_radius", 0)
        super().__init__(parent, *args, **kwargs)
        self.title = ctk.CTkLabel(
            self, text="Deciphon", font=ctk.CTkFont(size=24, weight="bold")
        )
        self.subtitle = ctk.CTkLabel(
            self, text="Protein annotation of long reads", font=ctk.CTkFont(size=14)
        )

        self.title.grid(row=0, column=0, sticky="w")
        self.subtitle.grid(row=1, column=0, sticky="w")

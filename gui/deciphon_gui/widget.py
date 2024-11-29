import customtkinter as ctk


def enable(x: ctk.CTkButton | ctk.CTkTextbox | ctk.CTkEntry | ctk.CTkOptionMenu):
    x.configure(state="normal")


def disable(x: ctk.CTkButton | ctk.CTkTextbox | ctk.CTkEntry | ctk.CTkOptionMenu):
    x.configure(state="disabled")


def get_state(x: ctk.CTkTextbox):
    return x.children["!text"].cget("state")

import customtkinter as ctk


def h1(master, text):
    return ctk.CTkLabel(master, text=text, font=ctk.CTkFont(size=20))


def h2(master, text):
    return ctk.CTkLabel(master, text=text, font=ctk.CTkFont(size=14, weight="bold"))


def h3(master, text: str, width: int = 0):
    return ctk.CTkLabel(master, text=text, width=width)

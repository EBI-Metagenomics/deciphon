import customtkinter as ctk


def h1(master, text):
    font = ctk.CTkFont(size=20)
    return ctk.CTkLabel(master, text=text, font=font)


def h2(master, text):
    font = ctk.CTkFont(size=14, weight="bold")
    return ctk.CTkLabel(master, text=text, font=font)


def h3(master, text: str, width: int = 0):
    return ctk.CTkLabel(master, text=text, width=width)

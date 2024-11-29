import customtkinter as ctk


def lift_window(window: ctk.CTk):
    window.attributes("-topmost", True)
    # get window on top
    window.update_idletasks()
    # prevent permanent focus
    window.attributes("-topmost", False)
    # focus to the window
    window.focus_force()

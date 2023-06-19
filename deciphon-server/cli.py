# from __future__ import annotations
# from deciphon.service_exit import service_exit
#
# import asyncio
# from typer import Exit, Option, Typer, echo, get_text_stream
#
# __all__ = ["app"]
#
#
# app = Typer(
#     add_completion=False,
#     pretty_exceptions_short=True,
#     pretty_exceptions_show_locals=False,
# )
#
#
# @app.command()
# def start(daemon: str):
#     """
#     Start `pressd` or `scand` daemons.
#     """
#     with service_exit():
#         if daemon == "pressd":
#             asyncio.run(deciphon.pressd.pressd())
#         if daemon == "scand":
#             asyncio.run(deciphon.scand.scand())

import sys

from loguru import logger

from deciphon_sched.settings import Settings


class Logger:
    def __init__(self, settings: Settings):
        logger.remove()
        logger.add(sys.stderr, level=settings.log_level.value.upper())

    @property
    def handler(self):
        return logger

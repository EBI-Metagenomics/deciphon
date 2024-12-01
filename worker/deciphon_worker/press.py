from functools import partial
from threading import Thread

from deciphon_core.press import PressContext
from deciphon_schema import DBFile, Gencode, HMMFile

from deciphon_worker.interrupted import Interrupted
from deciphon_worker.progressor import Progressor


def press_thread(
    future: Progressor[DBFile], hmmfile: HMMFile, gencode: Gencode, epsilon: float
):
    future.set_running_or_notify_cancel()
    try:
        with PressContext(hmmfile, gencode, epsilon) as ctx:
            future.set_progress(0)
            for i in range(ctx.nproteins):
                if future.interrupted:
                    raise Interrupted
                ctx.next()
                future.set_progress((100 * (i + 1)) // ctx.nproteins)

        future.set_result(DBFile(path=hmmfile.dbpath.path))
    except Exception as exception:
        future.set_exception(exception)


def press(hmmfile: HMMFile, gencode: Gencode, epsilon: float):
    future: Progressor[DBFile] = Progressor()
    thread = Thread(target=partial(press_thread, future, hmmfile, gencode, epsilon))
    thread.start()
    return future

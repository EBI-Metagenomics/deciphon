import typer

import deciphonctl.db as db
import deciphonctl.hmm as hmm
import deciphonctl.job as job
import deciphonctl.scan as scan
import deciphonctl.worker as worker

app = typer.Typer()
app.add_typer(hmm.app, name="hmm")
app.add_typer(db.app, name="db")
app.add_typer(job.app, name="job")
app.add_typer(scan.app, name="scan")
app.add_typer(worker.app, name="worker")

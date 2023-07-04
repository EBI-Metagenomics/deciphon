from pathlib import Path

import fire
import plotly.express as px
import polars as pl


def view_quantitatively_whole_genome(input: str, title: str = "notitle"):
    df = pl.read_parquet(Path(input))

    df = (
        df.groupby(["organism"])
        .agg(
            [
                pl.col("precision").mean(),
                pl.col("recall").mean(),
                pl.count(),
                pl.col("domain").first(),
            ]
        )
        .sort(by="domain", descending=True)
    )
    fig = px.scatter(
        df,
        x="precision",
        y="recall",
        color="domain",
        hover_data=[
            "precision",
            "recall",
            "organism",
            "count",
            "domain",
        ],
        marginal_x="histogram",
        marginal_y="histogram",
    )
    fig.update_layout(title=title)
    fig.update_xaxes(range=[-0.05, 1.05], row=1, col=1)
    fig.update_yaxes(range=[-0.05, 1.05], row=1, col=1)
    fig.show()


if __name__ == "__main__":
    fire.Fire(view_quantitatively_whole_genome)

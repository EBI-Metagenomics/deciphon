from pathlib import Path

import fire
import plotly.express as px
import polars as pl


def view_qualitatively(file: str, title: str = "qualitatively"):
    df = pl.read_parquet(Path(file))

    df = (
        df.group_by(["organism"])
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
    # fig.update_xaxes(range=[0.55, 1.05], row=1, col=1)
    # fig.update_yaxes(range=[0.75, 1.05], row=1, col=1)
    # fig.update_yaxes(range=[0.55, 1.05], row=1, col=1)
    # fig.update_xaxes(range=[-0.05, 1.05], row=1, col=1)
    # fig.update_yaxes(range=[-0.05, 1.05], row=1, col=1)
    fig.update_layout(font=dict(size=32))
    fig.update_traces(
        marker=dict(size=12, line=dict(width=2, color="DarkSlateGrey")),
        selector=dict(mode="markers"),
    )
    fig.write_html("qualitatively.html")


if __name__ == "__main__":
    fire.Fire(view_qualitatively)

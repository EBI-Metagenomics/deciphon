import pkgutil


def file_enabled_png():
    data = pkgutil.get_data("deciphon_gui", "file_enabled.png")
    assert data is not None
    return data


def file_disabled_png():
    data = pkgutil.get_data("deciphon_gui", "file_disabled.png")
    assert data is not None
    return data


def theme_json():
    data = pkgutil.get_data("deciphon_gui", "theme.json")
    assert data is not None
    return data

def _file_name_pattern(ext: str):
    return r"^[0-9a-zA-Z_\-.][0-9a-zA-Z_\-. ]+\." + ext + "$"


FILE_NAME_MAX_LENGTH = 128

HMM_FILE_NAME_PATTERN = _file_name_pattern("hmm")
DB_FILE_NAME_PATTERN = _file_name_pattern("dcp")
SNAP_FILE_NAME_PATTERN = _file_name_pattern("dcs")

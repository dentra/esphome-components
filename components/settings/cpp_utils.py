def fnv1_hash(data: str | bytes | bytearray) -> int:
    """FNV1 hash used in ESPHome"""
    FNV_OFFSET_BASIS = 2166136261
    FNV_PRIME = 16777619
    FNV_MASK = (2**32) - 1

    if isinstance(data, str):
        data = data.encode(encoding="utf-8")

    hash = FNV_OFFSET_BASIS
    for byte in data:
        hash = (hash * FNV_PRIME) & FNV_MASK
        hash ^= byte
    return hash

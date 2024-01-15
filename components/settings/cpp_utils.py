def fnv1_hash(s: str) -> int:
    h = 2166136261
    for c in s:
        h = (h * 16777619) % (2**32)
        h = h ^ ord(c)
    return h

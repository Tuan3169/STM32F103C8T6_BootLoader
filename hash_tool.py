import hashlib


def _zero_fields(data: bytearray, offsets: list[tuple[int, int]]) -> None:
    for offset, size in offsets:
        for i in range(size):
            data[offset + i] = 0x00


def embed_sha256(
    data: bytearray,
    hash_offset: int,
    hash_size: int,
) -> bytes:
    _zero_fields(data, [(hash_offset, hash_size)])
    digest = hashlib.sha256(data).digest()
    data[hash_offset:hash_offset + hash_size] = digest[:hash_size]
    return digest

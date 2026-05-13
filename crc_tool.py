import zlib


def _zero_fields(data: bytearray, offsets: list[tuple[int, int]]) -> None:
    for offset, size in offsets:
        for i in range(size):
            data[offset + i] = 0x00


def embed_crc32(
    data: bytearray,
    crc_offset: int,
    hash_offset: int,
    hash_size: int,
) -> int:
    _zero_fields(data, [(crc_offset, 4), (hash_offset, hash_size)])
    crc = zlib.crc32(data) & 0xFFFFFFFF
    data[crc_offset:crc_offset + 4] = crc.to_bytes(4, byteorder="little")
    return crc

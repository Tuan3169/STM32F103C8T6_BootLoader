import argparse
from pathlib import Path

from crc_tool import embed_crc32
from hash_tool import embed_sha256


def parse_int(value: str) -> int:
    return int(value, 0)


def update_hash(bin_path: Path, total_size: int, hash_offset: int, hash_size: int, crc_offset: int) -> None:
    data = bytearray(bin_path.read_bytes())

    if total_size <= 0:
        raise ValueError("total_size must be positive")
    if hash_size <= 0:
        raise ValueError("hash_size must be positive")
    if hash_offset < 0 or (hash_offset + hash_size) > total_size:
        raise ValueError("hash_offset/hash_size out of range")
    if crc_offset < 0 or (crc_offset + 4) > total_size:
        raise ValueError("crc_offset out of range")

    # Pad with 0xFF to the expected image size (matches erased flash default).
    if len(data) < total_size:
        data.extend(b"\xFF" * (total_size - len(data)))

    if len(data) > total_size:
        raise ValueError("binary larger than expected total_size")

    # Compute CRC32 and SHA256 with CRC/HASH fields zeroed to avoid self-inclusion.
    embed_crc32(data, crc_offset, hash_offset, hash_size)
    embed_sha256(data, hash_offset, hash_size, crc_offset)

    bin_path.write_bytes(data)


def main() -> int:
    parser = argparse.ArgumentParser(description="Embed CRC32 and SHA256 into binary")
    parser.add_argument("--bin", required=True, help="Path to app binary")
    parser.add_argument("--total-size", required=True, type=parse_int, help="Total image size (app + user_data)")
    parser.add_argument("--hash-offset", required=True, type=parse_int, help="Offset of hash field from image start")
    parser.add_argument("--hash-size", default="4", type=parse_int, help="Hash field size (default: 4)")
    parser.add_argument("--crc-offset", required=True, type=parse_int, help="Offset of CRC field from image start")
    args = parser.parse_args()

    update_hash(Path(args.bin), args.total_size, args.hash_offset, args.hash_size, args.crc_offset)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

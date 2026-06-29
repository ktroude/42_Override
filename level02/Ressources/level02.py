#!/usr/bin/env python3
"""Decode little-endian hex chunks leaked by the level02 format-string bug."""

chunks = [
    "756e505234376848",
    "45414a3561733951",
    "377a7143574e6758",
    "354a35686e475873",
    "48336750664b394d",
]

password = "".join(bytes.fromhex(chunk)[::-1].decode("latin-1") for chunk in chunks)
print(password)

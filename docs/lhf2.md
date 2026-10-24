# LHF2 single-file Huffman format

All integer fields are unsigned little-endian. The payload stores bits most significant bit first within each byte.

| Field | Size | Meaning |
| --- | --- | --- |
| Magic | 8 bytes | `4c 48 46 32 0d 0a 1a 0a` |
| Original length | uint64 | Number of original bytes |
| Payload bits | uint64 | Number of meaningful encoded bits |
| Original SHA-256 | 32 bytes | Binary digest, not hexadecimal text |
| Symbol count | uint16 | 0..256 |
| Frequency entries | count × 9 | byte symbol + uint64 nonzero frequency, ascending symbol order |
| Encoded payload | ceil(bits / 8) | Huffman-coded source; unused low bits in final byte are zero |

The tree is built by repeatedly merging the two smallest-frequency nodes. Nodes with equal frequency are ordered by their smallest contained symbol. First popped node is the left child (0); second is the right child (1). A single-symbol file uses code `0`; an empty input uses no symbols and no payload. Thus both extremes have a defined representation.

The decoder validates unique nonzero frequency entries, checked frequency sums, representable file lengths, reconstructed encoded bit count, exact remaining payload length, zero padding, decoded frequency counts and SHA-256. There are no nested archives, paths or executable metadata in the format. Decoding writes only to the output path explicitly chosen by the user and commits it only after validation.

Compression uses two streaming passes. The second pass recomputes source size, frequencies and SHA-256 to detect input changes; a changed source aborts before the destination is committed.

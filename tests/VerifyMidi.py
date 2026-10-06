"""Independent SMF decoder: validates export structure, timing and note pairing."""
import struct
import sys
from pathlib import Path


def verify(path):
    data = Path(path).read_bytes()
    assert data[:4] == b'MThd'
    size, fmt, count, ppq = struct.unpack('>IHHH', data[4:14])
    assert (size, fmt, count, ppq) == (6, 1, 10, 960)
    pos, note_count, marker_count = 14, 0, 0
    end_ticks = []
    for track_index in range(count):
        assert data[pos:pos+4] == b'MTrk'
        length = int.from_bytes(data[pos+4:pos+8], 'big')
        pos += 8
        end = pos + length
        assert end <= len(data)
        tick, active, ended, named = 0, {}, False, False

        def variable():
            nonlocal pos
            value = 0
            for _ in range(4):
                assert pos < end
                byte = data[pos]
                pos += 1
                value = (value << 7) | (byte & 127)
                if byte < 128:
                    return value
            raise AssertionError('Invalid variable-length quantity')

        while pos < end:
            tick += variable()
            status = data[pos]
            pos += 1
            if status == 255:
                kind = data[pos]
                pos += 1
                n = variable()
                payload = data[pos:pos+n]
                pos += n
                assert pos <= end
                if kind == 3:
                    named = bool(payload)
                elif kind == 6:
                    marker_count += 1
                elif kind == 81:
                    assert n == 3 and int.from_bytes(payload, 'big') > 0
                elif kind == 88:
                    assert n == 4 and payload[0] > 0
                elif kind == 47:
                    assert n == 0 and pos == end
                    ended = True
            else:
                assert status & 240 in (128, 144, 176)
                pitch, velocity = data[pos:pos+2]
                pos += 2
                assert pitch < 128 and velocity < 128
                if status & 240 == 176:
                    continue
                key = (status & 15, pitch)
                if status & 240 == 144 and velocity:
                    active[key] = active.get(key, 0) + 1
                    note_count += 1
                else:
                    assert active.get(key, 0) > 0, (track_index, 'unpaired note-off', key)
                    active[key] -= 1
        assert ended and named and not any(active.values())
        end_ticks.append(tick)
    assert pos == len(data) and note_count > 0 and marker_count >= 4
    assert len(set(end_ticks)) == 1
    print(f'PASS: independently decoded {count} tracks, {note_count} paired notes, {marker_count} markers; {end_ticks[0]/ppq:g} beats')


if __name__ == '__main__':
    verify(sys.argv[1])

#!/usr/bin/env python3
"""Convert video to raw RGB888 frames for the 16x16 ESP32 player.
Requires ffmpeg and ffprobe installed on PATH.
"""
import argparse
import shutil
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path, help='Source video, e.g. intro.mp4')
    parser.add_argument('-o', '--output', type=Path, help='Output .rgb path')
    parser.add_argument('--fps', type=int, default=15)
    parser.add_argument('--width', type=int, default=16)
    parser.add_argument('--height', type=int, default=16)
    args = parser.parse_args()
    if not args.input.is_file():
        parser.error(f'Input file does not exist: {args.input}')
    if not shutil.which('ffmpeg'):
        parser.error('ffmpeg is not installed or not on PATH')
    if min(args.fps, args.width, args.height) < 1:
        parser.error('fps, width and height must be positive')
    output = args.output or args.input.with_name(args.input.stem + f'_{args.width}x{args.height}.rgb')
    if output.resolve() == args.input.resolve():
        parser.error('Output must differ from input')
    # Explicit RGB24, row-major pixels, one frame after another, no header.
    filter_chain = f'fps={args.fps},scale={args.width}:{args.height}:flags=area'
    cmd = ['ffmpeg', '-hide_banner', '-y', '-i', str(args.input), '-an', '-vf', filter_chain,
           '-pix_fmt', 'rgb24', '-f', 'rawvideo', str(output)]
    subprocess.run(cmd, check=True)
    frame_size = args.width * args.height * 3
    total_bytes = output.stat().st_size
    if total_bytes % frame_size:
        raise RuntimeError('Output is not aligned to whole frames')
    print(f'Wrote {output}: {total_bytes // frame_size} frames, {total_bytes:,} bytes')


if __name__ == '__main__':
    main()

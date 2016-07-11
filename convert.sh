#!/bin/sh
ffmpeg -f rawvideo -vcodec rawvideo -s 1920x1080 -r 25 -pix_fmt rgb24 -i output.rgb -c:v libx264 -preset ultrafast -qp 2 -y output.mp4


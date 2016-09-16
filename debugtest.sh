#!/bin/sh
make &&
lldb-3.8 -O "break set -E c++" -- dist/view_syn $1

#!/bin/sh
make &&
dist/view_syn ../../poznan/conf.txt &&
../mf/tools/timeline.py timeline.json timeline.pdf

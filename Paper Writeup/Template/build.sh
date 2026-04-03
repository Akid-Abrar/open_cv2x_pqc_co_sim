#!/bin/bash
# Compile main.tex -> main.pdf (pdflatex + bibtex)
# Usage: ./build.sh        (incremental)
#        ./build.sh clean   (full rebuild)

cd "$(dirname "$0")"

if [ "$1" = "clean" ]; then
    rm -f main.aux main.bbl main.blg main.log main.out main.pdf
    echo "Cleaned build artifacts."
fi

pdflatex -interaction=nonstopmode main.tex > /dev/null 2>&1
bibtex main > /dev/null 2>&1
pdflatex -interaction=nonstopmode main.tex > /dev/null 2>&1
pdflatex -interaction=nonstopmode main.tex > /dev/null 2>&1

if [ -f main.pdf ]; then
    echo "Build successful: main.pdf ($(du -h main.pdf | cut -f1), $(pdfinfo main.pdf 2>/dev/null | grep Pages | awk '{print $2}') pages)"
else
    echo "Build FAILED. Check main.log for errors."
    exit 1
fi

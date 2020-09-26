@echo off
docker run --rm -v %CD%:/project -w /project espressif/idf idf.py build
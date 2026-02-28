#!/bin/bash
# Data download for HEMCO Example 4
mkdir -p data
./scripts/download_hemco_data.py HEMCO/MACCITY/v2014-10/MACCity_2000.nc -o data/$(basename HEMCO/MACCITY/v2014-10/MACCity_2000.nc)
./scripts/download_hemco_data.py HEMCO/GFED4/v2014-10/2000/GFED4_gen.nc -o data/$(basename HEMCO/GFED4/v2014-10/2000/GFED4_gen.nc)

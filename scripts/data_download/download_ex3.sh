#!/bin/bash
# Data download for HEMCO Example 3
mkdir -p data
./scripts/download_hemco_data.py HEMCO/MACCITY/v2014-10/MACCity_2000.nc -o data/$(basename HEMCO/MACCITY/v2014-10/MACCity_2000.nc)
./scripts/download_hemco_data.py HEMCO/AEIC/v2014-10/AEIC_2005.nc -o data/$(basename HEMCO/AEIC/v2014-10/AEIC_2005.nc)

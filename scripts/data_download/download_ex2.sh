#!/bin/bash
# Data download for HEMCO Example 2
mkdir -p data
./scripts/download_hemco_data.py HEMCO/MACCITY/v2014-10/MACCity_2000.nc -o data/$(basename HEMCO/MACCITY/v2014-10/MACCity_2000.nc)
./scripts/download_hemco_data.py HEMCO/EMEP/v2014-10/EMEP_2000.nc -o data/$(basename HEMCO/EMEP/v2014-10/EMEP_2000.nc)
./scripts/download_hemco_data.py HEMCO/MASKS/v2014-10/mask_europe.nc -o data/$(basename HEMCO/MASKS/v2014-10/mask_europe.nc)

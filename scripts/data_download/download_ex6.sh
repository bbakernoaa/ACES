#!/bin/bash
# Data download for HEMCO Example 6
mkdir -p data
./scripts/download_hemco_data.py HEMCO/EDGAR/v2014-10/EDGAR_v43.NOx.POW.nc -o data/$(basename HEMCO/EDGAR/v2014-10/EDGAR_v43.NOx.POW.nc)
./scripts/download_hemco_data.py HEMCO/CEDS/v2014-10/NO-em-anthro_CMIP_CEDS_2000.nc -o data/$(basename HEMCO/CEDS/v2014-10/NO-em-anthro_CMIP_CEDS_2000.nc)

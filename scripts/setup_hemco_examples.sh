#!/bin/bash
# setup_hemco_examples.sh
# Sets up ACES configuration files mimicking all 6 basic examples from HEMCO documentation.

set -e

mkdir -p examples
mkdir -p scripts/data_download

# Helper to generate download script for an example
generate_download_script() {
    local ex_num=$1
    local script_file="scripts/data_download/download_ex${ex_num}.sh"
    shift
    cat <<EOF > "$script_file"
#!/bin/bash
# Data download for HEMCO Example ${ex_num}
mkdir -p data
EOF
    for path in "$@"; do
        echo "./scripts/download_hemco_data.py $path -o data/\$(basename $path)" >> "$script_file"
    done
    chmod +x "$script_file"
}

# Example 1: Add global anthropogenic emissions (MACCity CO)
cat <<EOF > examples/aces_config_ex1.yaml
# ACES equivalent of HEMCO Example 1
meteorology:
  hourly_scalfact: HOURLY_SCALFACT

species:
  co:
    - field: "MACCITY_CO"
      operation: "add"
      scale: 1.0
      scale_fields: ["hourly_scalfact"]

cdeps_inline_config:
  streams:
    - name: "MACCITY_CO"
      file: "data/MACCity_4x5.nc"
    - name: "HOURLY_SCALFACT"
      file: "data/hourly.nc"
EOF
generate_download_script 1 "HEMCO/MACCITY/v2014-07/MACCity_4x5.nc"

# Example 2: Overlay regional emissions
cat <<EOF > examples/aces_config_ex2.yaml
# ACES equivalent of HEMCO Example 2
meteorology:
  hourly_scalfact: HOURLY_SCALFACT
  mask_europe: MASK_EUROPE

species:
  co:
    - field: "MACCITY_CO"
      category: "1"
      hierarchy: 1
      operation: "add"
      scale_fields: ["hourly_scalfact"]
    - field: "EMEP_CO"
      category: "1"
      hierarchy: 2
      operation: "replace"
      mask: "mask_europe"
      scale_fields: ["hourly_scalfact"]

cdeps_inline_config:
  streams:
    - name: "MACCITY_CO"
      file: "data/MACCity_4x5.nc"
    - name: "EMEP_CO"
      file: "data/EMEP_2000.nc"
    - name: "HOURLY_SCALFACT"
      file: "data/hourly.nc"
    - name: "MASK_EUROPE"
      file: "data/mask_europe.nc"
EOF
generate_download_script 2 "HEMCO/MACCITY/v2014-07/MACCity_4x5.nc" "HEMCO/EMEP/v2014-07/EMEP_2000.nc" "HEMCO/MASKS/v2014-07/mask_europe.nc"

# Example 3: Adding the AEIC aircraft emissions
cat <<EOF > examples/aces_config_ex3.yaml
# ACES equivalent of HEMCO Example 3
species:
  co:
    - field: "MACCITY_CO"
      category: "1"
      hierarchy: 1
      operation: "add"
    - field: "AEIC_CO"
      category: "2"
      hierarchy: 1
      operation: "add"

cdeps_inline_config:
  streams:
    - name: "MACCITY_CO"
      file: "data/MACCity_4x5.nc"
    - name: "AEIC_CO"
      file: "data/AEIC.nc"
EOF
generate_download_script 3 "HEMCO/MACCITY/v2014-07/MACCity_4x5.nc" "HEMCO/AEIC/v2015-01/AEIC.nc"

# Example 4: Add biomass burning emissions
cat <<EOF > examples/aces_config_ex4.yaml
# ACES equivalent of HEMCO Example 4
physics_schemes:
  - name: "GFED"
    language: "cpp"
    options:
      version: "GFED4"

species:
  co:
    - field: "MACCITY_CO"
      category: "1"
      hierarchy: 1
      operation: "add"

cdeps_inline_config:
  streams:
    - name: "MACCITY_CO"
      file: "data/MACCity_4x5.nc"
    - name: "GFED_WDL"
      file: "data/GFED4_gen.nc"
EOF
generate_download_script 4 "HEMCO/MACCITY/v2014-07/MACCity_4x5.nc"

# Example 5: Additional species
cat <<EOF > examples/aces_config_ex5.yaml
# ACES equivalent of HEMCO Example 5
species:
  co:
    - field: "MACCITY_CO"
      operation: "add"
  no:
    - field: "MACCITY_NO"
      operation: "add"
  so2:
    - field: "MACCITY_SO2"
      operation: "add"

cdeps_inline_config:
  streams:
    - name: "MACCITY_CO"
      file: "data/MACCity_4x5.nc"
    - name: "MACCITY_NO"
      file: "data/MACCity_anthro_NOx_2000-2010_16080.nc"
    - name: "MACCITY_SO2"
      file: "data/MACCity_anthro_SO2_2000-2010_16080.nc"
EOF
generate_download_script 5 "HEMCO/MACCITY/v2014-07/MACCity_4x5.nc" "HEMCO/MACCITY/v2014-07/MACCity_anthro_NOx_2000-2010_16080.nc" "HEMCO/MACCITY/v2014-07/MACCity_anthro_SO2_2000-2010_16080.nc"

# Example 6: Non-separated inventories
cat <<EOF > examples/aces_config_ex6.yaml
# ACES equivalent of HEMCO Example 6
species:
  no:
    - field: "EDGAR_NO_POW"
      category: "1"
      operation: "add"
    - field: "CEDS_NO_AGR"
      category: "1"
      operation: "add"

cdeps_inline_config:
  streams:
    - name: "EDGAR_NO_POW"
      file: "data/EDGAR_v43.NOx.POW.nc"
    - name: "CEDS_NO_AGR"
      file: "data/NO-em-anthro_CMIP_CEDS.nc"
EOF
generate_download_script 6 "HEMCO/EDGARv43/v2014-10/EDGAR_v43.NOx.POW.nc" "HEMCO/CEDS/v2014-10/NO-em-anthro_CMIP_CEDS.nc"

echo "All 6 ACES example configurations created in the 'examples/' directory."
echo "Data download scripts created in 'scripts/data_download/'."

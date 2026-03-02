# ACES Developer Guide

## Project Overview
ACES (Accelerated Component for Emission System) is a C++20 emissions compute component designed for high performance using Kokkos and ESMF.

## Global Coding Standards
*   **Language:** C++20.
*   **Style:** Google C++ Style Guide.
*   **Namespace:** `aces::` (defined in `include/aces/aces.hpp`).
*   **Documentation:** Doxygen format (`/** ... */`) required for all public APIs.
*   **Memory:** Use `Kokkos::View` for data. Avoid raw pointers.
*   **ESMF:** Use `ESMC_` C API. Wrap data in `Kokkos::View` with `Kokkos::MemoryTraits<Kokkos::Unmanaged>`.
*   **Performance Portability:** All compute kernels MUST use Kokkos parallel primitives and avoid hardware-specific code.

## Development Environment
The required development environment is the `jcsda/docker-gnu-openmpi-dev:1.9` Docker container. This container provides the necessary compilers, MPI, and ESMF dependencies.

### ⚠️ IMPORTANT: No Mocking Policy
**Mocking ESMF or NUOPC is strictly forbidden.** All development and verification must be performed using real ESMF dependencies within the JCSDA Docker environment to ensure real-world parity and stability.

### Quick Start
1.  **Run the Setup Script:**
    Execute the provided `setup.sh` script to pull the Docker image and drop into a shell.
    ```bash
    ./setup.sh
    ```
    If you encounter Docker overlayfs issues or need to fix the environment, run:
    ```bash
    ./scripts/fix_docker_and_setup.sh
    ```

2.  **Build:**
    Inside the container:
    ```bash
    mkdir build && cd build
    cmake ..
    make -j4
    ```

 3.  **Run Example Driver:**
    To see ACES in action with external data (ESMF fields):
    ```bash
    ./example_driver
    ```

4.  **Test:**
    ```bash
    ctest --output-on-failure
    ```

### Python Dependencies
The physics scheme generator and other scripts require `jinja2`, `pyyaml`, and `pytest`.
```bash
python3 -m pip install jinja2 pyyaml pytest
```

#!/usr/bin/env python3
import yaml
import sys
import os
from jinja2 import Environment, FileSystemLoader

def generate_scheme(config_path):
    if not os.path.exists(config_path):
        print(f"Error: Config file {config_path} not found.")
        sys.exit(1)

    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)

    env = Environment(loader=FileSystemLoader('scripts/templates'))

    class_name = config.get('class_name', 'MyPhysicsScheme')
    scheme_name = config.get('scheme_name', 'my_scheme')
    description = config.get('description', 'Auto-generated physics scheme.')
    header_name = f"aces_{scheme_name}.hpp"

    context = {
        'class_name': class_name,
        'scheme_name': scheme_name,
        'description': description,
        'header_name': header_name,
        'imports': config.get('imports', []),
        'exports': config.get('exports', [])
    }

    # Generate Header
    hpp_template = env.get_template('physics_scheme.hpp.jinja2')
    with open(f"include/aces/physics/{header_name}", 'w') as f:
        f.write(hpp_template.render(context))

    # Generate Implementation
    cpp_template = env.get_template('physics_scheme.cpp.jinja2')
    with open(f"src/physics/aces_{scheme_name}.cpp", 'w') as f:
        f.write(cpp_template.render(context))

    # Generate Fortran Kernel (optional)
    if config.get('generate_fortran', False):
        f90_template = env.get_template('physics_kernel.F90.jinja2')
        with open(f"src/physics/{scheme_name}_kernel.F90", 'w') as f:
            f.write(f90_template.render(context))

    print(f"Successfully generated scheme: {class_name} ({scheme_name})")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: ./generate_physics_scheme.py <config.yaml>")
        sys.exit(1)
    generate_scheme(sys.argv[1])

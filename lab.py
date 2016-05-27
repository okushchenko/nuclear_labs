#!/usr/bin/env python
from jinja2 import Environment, FileSystemLoader
import subprocess
import os

lab_name = "B1"

params_detector = {}
params_detector["environment_material"] = os.getenv("environment_material", "AIR")
params_detector["world_material"] = os.getenv("world_material", "AIR")
params_detector["array_material"] = os.getenv("array_material", "Pb")
params_detector["detector_material"] = os.getenv("detector_material", "Pb")
params_detector["detector_size"] = float(os.getenv("detector_size", 2.5)) # in cm

params_generator = {}
params_generator["particle_energy"] = float(os.getenv("particle_energy", 6.0)) # in MeV

files = {
            "B1DetectorConstruction.cc": params_detector,
            "B1PrimaryGeneratorAction.cc": params_generator
        }

for name, args in files.iteritems():
    env = Environment(loader=FileSystemLoader('{0}/src'.format(lab_name)))
    template = env.get_template(name + ".j2")
    output_from_parsed_template = template.render(**args)
    print output_from_parsed_template
    # to save the results
    with open('{0}/src/'.format(lab_name) + name, "wb") as fh:
        fh.write(output_from_parsed_template)

subprocess.call("./build_and_run.sh")

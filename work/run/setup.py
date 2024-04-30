####################################################################
#   rvfpm - 2024
#   Andreas S. Bakke
#
#   Description:
#   Uses default_config.yaml and other user defined yaml files to
#   to write config.svh and config.h to ensure that the same
#   parameters are used for both systemverilog and cpp.
######################################################################

import sys
import os
import yaml

#TODO: add path variable to enable run from multiple folders
sv_path = "src/config.svh"
cpp_path = "include/config.h"

header  = "//####################################################################\n"
header += "// rvfpm - 2024\n"
header += "// Andreas S. Bakke\n"
header += "//\n"
header += "// DO NOT MODIFY\n"
header += "// AUTOMATICALLY OVERWRITTEN BY run/setup.py\n"
header += "//####################################################################\n\n"

def write_header(path):
  try:
    with open(path, 'w+', encoding='utf-8') as file:
      file.write(header)
  except Exception as e:
    print(f"Failed: {e}")

def write_data(path, data):
  try:
    with open(path, 'a', encoding='utf-8') as file:
      file.write(data)
  except Exception as e:
    print(f"Failed: {e}")

def write_both(data, mode="a"):
  try:
    with open(sv_path, mode, encoding='utf-8') as file:
      file.write(data)
  except Exception as e:
    print(f"Failed: {e}")
  try:
    with open(cpp_path, mode, encoding='utf-8') as file:
      file.write(data)
  except Exception as e:
    print(f"Failed: {e}")


def write_sv_params(data):
  try:
    write_data(sv_path, "\n//System-parameters\n")
    for param in data["fpu"]:
      if param == "extensions":
        for extension in data["fpu"][param]:
          write_data(sv_path, "  `define "+str(extension).upper()+"\n")
        continue
      write_data(sv_path, "  `define "+str(param).upper()+" "+str(data["fpu"][param])+"\n")
  except Exception as e:
    print(f"Failed writing sv system-parameters: {e}")

  try:
    write_data(sv_path, "\n//CORE-V-XIF-parameters\n")
    for param in data["xif"]:
      write_data(sv_path, "  `define "+str(param).upper()+" "+str(data["xif"][param]).lower()+"\n")
  except Exception as e:
    print(f"Failed writing sv interface-parameters: {e}")

  try:
    write_data(sv_path, "\n//Pipeline-parameters\n")
    for param in data["fpu_pipeline"]:
      if param == "num_pipeline_stages" and data["fpu_pipeline"][param] != 0:
        write_data(sv_path, "  `define INCLUDE_PIPELINE\n")
      if param == "queue_depth" and data["fpu_pipeline"][param] != 0:
        write_data(sv_path, "  `define INCLUDE_QUEUE\n")

      if param == "steps":  #Not needed in pa_rvfpm as of yet
        continue
      if data["fpu_pipeline"][param] == True:
        write_data(sv_path, "  `define "+str(param).upper()+" 1\n")
      elif data["fpu_pipeline"][param] == False:
        write_data(sv_path, "  `define "+str(param).upper()+" 0\n")
      else:
        write_data(sv_path, "  `define "+str(param).upper()+" "+str(data["fpu_pipeline"][param]).lower()+"\n")
  except Exception as e:
    print(f"Failed writing sv pipeline-parameters: {e}")

  try:
    write_data(sv_path, "\n//Other\n")
    for param in data["defines"]:
      write_data(sv_path, "  `define "+str(param).upper()+"\n")
  except Exception as e:
    print(f"Failed writing sv defines: {e}")


def write_cpp_params(data):
  try:
    write_data(cpp_path, "\n//System-parameters\n")
    for param in data["fpu"]:
      if param == "extensions":
        for extension in data["fpu"][param]:
          write_data(cpp_path, "#define "+str(extension).upper())
        continue
      write_data(cpp_path, "const int "+str(param).upper()+"="+str(data["fpu"][param])+";\n")
  except Exception as e:
    print(f"Failed writing cpp system-parameters: {e}")

  try:
    write_data(cpp_path, "\n//CORE-V-XIF-parameters\n")
    for param in data["xif"]:
      write_data(cpp_path, "const int "+str(param).upper()+"="+str(data["xif"][param]).lower()+";\n")
  except Exception as e:
    print(f"Failed writing cpp interface-parameters: {e}")

  try:
    write_data(cpp_path, "\n//Pipeline-parameters\n")
    for param in data["fpu_pipeline"]:
      if param == "steps":
        write_data(cpp_path, "\nenum pipelineConfig {\n") if len(data["fpu_pipeline"]["steps"])>0 else None
        for step in data["fpu_pipeline"]["steps"]:
          write_data(cpp_path, "  "+str(step).upper()+" = "+str(data["fpu_pipeline"]["steps"][step])+",\n")
        write_data(cpp_path, "};\n") if len(data["fpu_pipeline"]["steps"])>0 else None
        continue
      if param == "hazards":
        for hazard in data["fpu_pipeline"]["hazards"]:
          write_data(cpp_path, "#define CTRL_"+str(hazard).upper()+"\n")
        continue
      if data["fpu_pipeline"][param] == True:
        write_data(cpp_path, "const int "+str(param).upper()+"=1;\n")
        if param == "forwarding" or param == "OOO":
          write_data(cpp_path, "#define "+str(param).upper()+"\n")
      elif data["fpu_pipeline"][param] == False:
        write_data(cpp_path, "const int "+str(param).upper()+"=0;\n")
      else:
        write_data(cpp_path, "const int "+str(param).upper()+"="+str(data["fpu_pipeline"][param]).lower()+";\n")
  except Exception as e:
    print(f"Failed writing cpp pipeline-parameters: {e}")

  try:
    write_data(cpp_path, "\n//Ex-cycles-parameters\n")
    for param in data["execute_cycles"]:
      write_data(cpp_path, "#define NUM_CYCLES_"+str(param).upper()+" "+str(data["execute_cycles"][param]).lower()+"\n")
  except Exception as e:
    print(f"Failed writing cpp ex-cycles-parameters: {e}")


  try:
    write_data(cpp_path, "\n//Other\n")
    for param in data["defines"]:
      write_data(cpp_path, "#define "+str(param).upper()+"\n")
  except Exception as e:
    print(f"Failed writing cpp defines: {e}")



def recursive_update(default, overwrite):
  for key, value in overwrite.items():
    if isinstance(value, dict) and key in default:
      recursive_update(default[key], value)
    else:
      default[key] = value


def parse_yaml(yaml_path):
  #load default config
  try:
    with open("run/default_config.yaml", 'r') as file:
      config = yaml.load(file, Loader=yaml.FullLoader)
  except Exception as e:
    print(f"Failed parsing default_config: {e}")

  if yaml_path != "":
    try:
      with open(yaml_path, 'r') as file:
        user_config = yaml.load(file, Loader=yaml.FullLoader)
    except Exception as e:
      print(f"Failed parsing {yaml_path}: {e}")

    recursive_update(config, user_config)
  return config



if __name__ == "__main__":
  yaml_path = sys.argv[1] if len(sys.argv) > 1 else ""
  print("\n---------------------------")
  print("-- Started setup parsing --")
  print("---------------------------\n")
  print(f"  User config given: {yaml_path}\n  Overwriting default_config.yaml") if yaml_path != "" else print("  No user config given, using default_config.yaml\n")

  write_both(header, "w+")
  # write_data(sv_path, "package pa_defines;\n")
  write_data(sv_path, "`ifndef MY_DEFINES_SV\n`define MY_DEFINES_SV")
  write_data(cpp_path, "#pragma once\n")
  config = parse_yaml(yaml_path)
  write_sv_params(config)
  write_cpp_params(config)
  write_data(sv_path, "`endif")
  write_both("\n\n", "a")
  # write_data(sv_path,"endpackage: pa_defines")

  print("----------------------------")
  print("-- Finished setup parsing --")
  print("----------------------------\n")


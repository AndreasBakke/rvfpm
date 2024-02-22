####################################################################
#   rvfpm - 2024
#   Andreas S. Bakke
#
#   Description:
#   Uses fpu_config.yaml to write pa_rvfpm.sv and config.h to ensure
#   that the same parameters are used for both
######################################################################


#Notes:
#Header: DO NOT MODIFY - AUTOMATICALLY WRITTEN...
import sys
import os

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

def write_sv():
  sv_path = "../src/test.sv"
  write_header(sv_path)

  write_data(sv_path, "package pa_rvfpm;\n")
    #Add parameters based on fpu_config.yaml
  write_data(sv_path, "endpackage: pa_rvfpm;")


if __name__ == "__main__":
  write_sv()







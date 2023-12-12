import sys
import os
import re
results = {}

total_errors = 0

def get_results(arch):
    for test_dir in os.listdir("./tests/" + arch):
        results[test_dir] = {}
        for file in os.listdir("./tests/" + arch + "/" + test_dir + "/"):
            file_path = os.path.join("./tests/", arch, test_dir, file)
            try:
                results[test_dir][file[:-4]] = extract_errors(file_path)
            except Exception as e:
                print(f"Error reading {filename}: {e}")
            # print(results)
def extract_errors(path):
    with open(path, 'r', encoding='utf-8') as file:
        line = file.readlines()[-1] #get last line
        print(path + re.search(r"([1-9]+) tests", line))
        tests_performed = 0#TODO:regex extraction   In 46464 tests, no errors found in f32_div, rounding near_even.  /  6133248 tests performed; 5964 errors found.
        errors = 0 #TODO:regex extraction  
        return {"tests_performed": tests_performed, "errors": errors}

def summarize_tests(results):
    print("haw")


architecture = ""

if __name__ == "__main__":
    for arg in sys.argv:
        if arg[0:6] == "-arch=":
            architecture = arg[6:]

    get_results(architecture)
        #Add more args if necessary
            

#         import os

# def read_last_lines(file_path, number_of_lines=3):
#     """Read the last n lines from a file."""
#     with open(file_path, 'r', encoding='utf-8') as file:
#         lines = file.readlines()
#         return lines[-number_of_lines:]

# def process_files_in_folder(folder_path):
#     """Process each text file in the specified folder."""
#     for filename in os.listdir(folder_path):
#         if filename.endswith('.txt'):
#             file_path = os.path.join(folder_path, filename)
#             try:
#                 last_lines = read_last_lines(file_path)
#                 print(f"Last lines of {filename}:")
#                 for line in last_lines:
#                     print(line.strip())
#                 print("\n")
#             except Exception as e:
#                 print(f"Error reading file {filename}: {e}")

# # Replace 'your_folder_path' with the path of your folder
# folder_path = 'your_folder_path'
# process_files_in_folder(folder_path)

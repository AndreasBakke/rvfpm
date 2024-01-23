####################################################################
#     rvfpm - 2023
#     Andreas S. Bakke
#    
#     Description:
#     Script for parsing results from the Berkeley TestFloat suite. 
#     Only works if called from work/
######################################################################
import sys
import os
import re
results = {}
architecture = ""
total_errors = 0

def get_results(arch):
    for test_dir in os.listdir("./tests/" + arch):
        results[test_dir] = {}
        for file in os.listdir("./tests/" + arch + "/" + test_dir + "/"):
            file_path = os.path.join("./tests/", arch, test_dir, file)
            try:
                results[test_dir][file[:-4]] = extract_errors(file_path)
            except Exception as e:
                print(f"Error reading {file_path}: {e}")
    summarize_tests(results)

def extract_errors(path):
    with open(path, 'r', encoding='utf-8') as file:
        tests_performed = 0
        errors = 0
        line = file.readlines()[-1] #get last line
        t = re.search(r'([0-9]+)\ tests', line)
        if t != None:
            tests_performed = t.groups()[0]
        e = re.search(r'([0-9]+)\ errors found', line)
        if e != None:
            errors= e.groups()[0]
        
        return {"tests_performed": tests_performed, "errors": errors}

def summarize_tests(results):
    instructions_passed = 0
    instructions_failed = 0
    failed_tests = {}
    total_tests_performed = 0
    total_tests_failed = 0
    for rm in results:
        failed_tests[rm] = {}
        for test in results[rm]:
            total_tests_performed += int(results[rm][test]["tests_performed"])
            if results[rm][test]["errors"] != 0:
                instructions_failed +=1
                failed_tests[rm][test] = results[rm][test]
                total_tests_failed += int(results[rm][test]["errors"])
            else:
                instructions_passed +=1
    writeSummary(results, total_tests_performed, total_tests_failed)
    print("\n\n########################################")
    print("############ TEST COMPLETED ############")
    print("########################################")
    print(f"TESTS PERFORMED: {total_tests_performed:,}")
    print(f"TESTS FAILED: {total_tests_failed:,}\n\n")


    print(f"A total of {total_tests_performed:,} tests for {instructions_failed + instructions_passed} total instructions using {len(results)} rounding modes was performed.")
    print(f"{total_tests_failed:,} tests failed.\n")
    if (total_tests_failed != 0):
        print(f"The following instructions failed:")
        for rm in failed_tests:
            print(f"    {rm}")
            for test in failed_tests[rm]:
                print(f"       {test}: Tests performed = {int(failed_tests[rm][test]['tests_performed']):,}, tests failed = {int(failed_tests[rm][test]['errors']):,}")

        print(f"\n For details, refer to summary in tests/summary_{architecture}.txt or the individual tests in: tests/{architecture}/<rounding mode>/<test>.txt\n")
    print("########################################")
    print("########################################")

def writeSummary(results, total_tests_performed, total_tests_failed):
    file_path = "./tests/summary_" + architecture + ".txt"
    try: 
        with open(file_path, 'w+', encoding='utf-8') as file:
            writeHeader(file, total_tests_performed, total_tests_failed)
    except Exception as e:
        print(f"Failed to create file or header. Error: {e}")
        return

    for rm in results:
        try: 
            with open(file_path, 'a', encoding='utf-8') as file:
                    header = "#### ROUNDING MODE: " + rm + " ####"
                    sep = "#"*len(header)
                    file.write(f"\n{sep}\n")
                    file.write(header)
                    file.write(f"\n{sep}\n")

        except Exception as e:
            print(f"Failed to write rounding mode to file. Error: {e}")
        for test in results[rm]:
            try:
                with open(file_path, 'a', encoding='utf-8') as file:
                    file.write(f"   {test}: Tests performed = {int(results[rm][test]['tests_performed']):,}, tests failed = {int(results[rm][test]['errors']):,}\n")

            except Exception as e:
                print(f"Failed to write test-result to file. Error: {e}")

def writeHeader(file, total_tests_performed, total_tests_failed):
    file.write("#####################################\n")
    file.write("######### TESTFLOAT SUMMARY #########\n")
    file.write("#####################################\n\n")
    file.write(f"TESTS PERFORMED: {total_tests_performed:,}\n")
    file.write(f"TESTS FAILED: {total_tests_failed:,}\n\n")
    file.write("#####################################\n")
    file.write("############# ALL TESTS #############\n")
    file.write("#####################################\n")


    


if __name__ == "__main__":
    for arg in sys.argv:
        if arg[0:6] == "-arch=":
            architecture = arg[6:]
    get_results(architecture)

import glob

def compare_files(my_result_path, test_result_path):
    with open(my_result_path, 'r') as my_result, open(test_result_path, 'r') as test_result:
        content1 = my_result.read();
        content2 = test_result.read();

    if content1 == content2:
        # print(f"{my_result_path} is clear!!")
        pass
    else:
        print(f"{my_result_path} and {test_result_path} is error!!!!")

my_result_dir = [file for file in glob.glob("./my_result/*")]
my_result_dir.sort()
test_result_dir = [file for file in glob.glob("./testresult/*") if not file.startswith('./testresult/test_')]
test_result_dir.sort()

# print(my_result_dir)
# print(test_result_dir)

for i in range(len(my_result_dir)):
    compare_files(my_result_dir[i], test_result_dir[i])


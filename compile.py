import os

if __name__ == "__main__":
    with open("all_files.txt", "w") as all_fp:
        files = os.listdir(".")
        for file in files:
            if os.path.isfile(file) and file.endswith(".c"):
                print(file)
                all_fp.writelines([
                    "****************************************************\n",
                    "**************** {file} *************************\n".format(file=file),
                    "****************************************************\n",
                ])
                with open(file, "r") as fp:
                    all_fp.writelines(fp.readlines())

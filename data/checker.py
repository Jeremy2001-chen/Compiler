import os
import shutil

if not os.path.exists("test"):
    os.mkdir("test")
# os.system("g++ ./Compiler/main.cpp -O2 -o compiler.exe")
shutil.move("./main", "./test/compiler")
os.chdir("./test")

num = 30
accept = 0
notAccept = []
for i in range(1, num + 1):
    suffix = str(i) + ".txt"
    inputName = "input" + suffix
    sourceName = "testfile" + suffix
    targetName = "ir" + suffix
    myName = "my" + suffix
    stdName = "output" + suffix
    mipsName = "mips" + suffix
    mipsOutputName = "mips_output" + suffix
    shutil.copy(sourceName, "testfile.txt")
    # shutil.copy("mips.txt", mipsName)
    ret = os.system("./compiler")
    if ret != 0:
        exit(ret)
    shutil.copy("ir.txt", targetName)
    shutil.copy("./" + inputName, "./input.txt")
    # ret_ir = os.system("java -jar Compiler-IR.jar <" + targetName + " > " + myName)
    ret_mips = os.system("java -jar Mars-Compile-2021.jar mips.txt < " + inputName +
                         " > " + mipsOutputName)
    now = 0
    tmpFile = open("tmp.txt", 'w')
    with open(mipsOutputName) as f:
        while True:
            lines = f.readline()
            now = now + 1
            if not lines:
                break
            if now > 2:
                tmpFile.write(lines)
    tmpFile.close()
    shutil.copy("tmp.txt", mipsOutputName)

    if ret_mips != 0:
        print("Execute Error at test %d" % i)
        notAccept.append(i)
    else:
        diff = os.system("diff " + stdName + " " + mipsOutputName)
        # fc_ir = os.system("diff " + myName + " " + stdName)
        # fc_mips = os.system("diff " + myName + " " + mipsOutputName)
        if diff != 0:
            print("Wrong Answer at test %d" % i)
            notAccept.append(i)
        else:
            accept = accept + 1

if accept == num:
    print("OK you Accept!\n")
else:
    print("No you Not Accept!\n")
    print(notAccept)
    exit(1)

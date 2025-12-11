import os
import subprocess
import glob

# === 配置区域 ===
PROGRAM = "build/code"       # Windows 请改为 "Bookstore.exe"
TEST_DIR = "test/TestPoints"            # 存放 .in 和 .out 的文件夹
TMP_DIR = "test/tmp_ans"           # 存放你的 .out 和 .err 的文件夹
START_ID = 200
END_ID = 209


def get_test_files():
    all_files = glob.glob(os.path.join(TEST_DIR, "*.in"))
    valid_files = []
    for f in all_files:
        basename = os.path.basename(f)
        name_no_ext = os.path.splitext(basename)[0]
        if name_no_ext.isdigit():
            idx = int(name_no_ext)
            if START_ID <= idx <= END_ID:
                valid_files.append((idx, f))
    valid_files.sort(key=lambda x: x[0])
    return [f[1] for f in valid_files]

def run_test():
    if not os.path.exists(TMP_DIR):
        os.makedirs(TMP_DIR)

    in_files = get_test_files()
    if not in_files:
        print("未找到测试文件")
        return

    print(f"🚀 开始测试 ID {START_ID}-{END_ID}")
    print("-" * 60)

    for in_file in in_files:
        test_id = os.path.basename(in_file) # "7.in"
        ans_file = in_file.replace(".in", ".out")
        
        # 定义输出和错误日志的路径
        base_name = test_id.replace(".in", "")
        my_out_path = os.path.join(TMP_DIR, base_name + ".out")
        my_err_path = os.path.join(TMP_DIR, base_name + ".err") # 【新】错误日志

        if not os.path.exists(ans_file):
            print(f"⚠️  {test_id}: 缺少答案文件，跳过")
            continue

        with open(in_file, 'r', encoding='utf-8') as fin:
            try:
                res = subprocess.run(
                    [PROGRAM], 
                    stdin=fin, 
                    capture_output=True, # 同时捕获 cout 和 cerr
                    text=True,
                    timeout=5 
                )
            except subprocess.TimeoutError:
                print(f"❌ {test_id}: 超时")
                print("🛑 停止测试")
                break
            except Exception as e:
                print(f"❌ {test_id}: 无法运行 ({e})")
                print("🛑 停止测试")
                break

        # 1. 保存标准输出 (cout)
        with open(my_out_path, 'w', encoding='utf-8') as f:
            f.write(res.stdout)

        # 2. 【新】保存错误输出 (cerr)
        with open(my_err_path, 'w', encoding='utf-8') as f:
            f.write(res.stderr)

        # 3. 比对答案
        my_lines = [line.strip() for line in res.stdout.splitlines() if line.strip()]
        
        with open(ans_file, 'r', encoding='utf-8') as fans:
            ans_lines = [line.strip() for line in fans.read().splitlines() if line.strip()]

        if my_lines == ans_lines:
            print(f"✅ {test_id}: AC")
        else:
            print(f"❌ {test_id}: WA")
            print(f"   你的输出: {my_out_path}")
            # 如果有错误信息，提示用户去看
            if len(res.stderr) > 0:
                print(f"   调试信息: {my_err_path} (检测到 cerr 内容)")
            print("🛑 停止测试")
            break

    print("-" * 60)

if __name__ == "__main__":
    run_test()
import os
import shutil

SRC_DIR = "src"
DST_DIR = "."
CONFIG_FILE = "ProjectConfig.sdl"

# đọc config
config = {}
with open(CONFIG_FILE) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        key, val = line.split("=")
        config[key.strip()] = val.strip()

# tạo thư mục build nếu cần
os.makedirs(DST_DIR, exist_ok=True)

# replace token trong các file hiện có
for root, dirs, files in os.walk("."):
    # bỏ qua build và các thư mục không cần
    if root.startswith("./build") or root.startswith("./.git"):
        continue

    for file in files:
        if not file.endswith((".c", ".h", ".ld", ".mk", "Makefile")):
            continue

        file_path = os.path.join(root, file)

        with open(file_path, "r") as f:
            content = f.read()

        # replace tất cả token
        modified = False
        for k, v in config.items():
            old_content = content
            content = content.replace(f"${{{k}}}", v)
            if content != old_content:
                modified = True

        # chỉ ghi lại nếu có thay đổi
        if modified:
            with open(file_path, "w") as f:
                f.write(content)
            print(f"Updated: {file_path}")

print("Token replacement completed.")
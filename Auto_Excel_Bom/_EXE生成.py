# build_exe.py
import os
import sys
import PyInstaller.__main__

def main():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    script_path = os.path.join(current_dir, "gui_app.py")  # 你的主入口文件

    if not os.path.exists(script_path):
        print(f"✗ 未找到主程序文件: {script_path}")
        sys.exit(1)

    # 在当前目录下创建 build 和 dist 文件夹
    dist_path = os.path.join(current_dir, "dist")
    build_path = os.path.join(current_dir, "build")

    args = [
        script_path,
        "--name=微现BOM表处理工具",
        "--onefile",         # 单文件exe
        "--windowed",        # GUI程序不弹控制台
        "--clean",
        "--noconfirm",
        
        # 指定输出路径为当前目录
        f"--distpath={dist_path}",
        f"--workpath={build_path}",
        
        # 常见隐藏导入（避免部分环境运行时报错）
        "--hidden-import=openpyxl",
        "--hidden-import=pandas",
        "--hidden-import=numpy",
    ]

    # 如果你有图标，可取消注释并替换路径
    # icon_path = os.path.join(current_dir, "app.ico")
    # if os.path.exists(icon_path):
    #     args.append(f"--icon={icon_path}")

    PyInstaller.__main__.run(args)

    print(f"\n✓ 打包完成！exe 文件在 {dist_path} 文件夹中")
    print("✓ 若被杀软拦截，建议加入白名单后再试")

if __name__ == "__main__":
    main()
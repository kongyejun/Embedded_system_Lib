import os
import threading
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
from tkinter import ttk

from bom_generator import BOMGenerator
from bom_match import BOMMatcher
from log_system import AppLogger, LogLevel


class ExcelProcessorGUI:
    """界面与调度"""

    DEFAULT_POOL_FILE = r"E:\MyFirmCode\BOM快捷生成表软件\电子料汇总表-V1.0.xlsx"

    def __init__(self, root):
        self.root = root
        self.root.title("微现BOM表处理工具 v1.0")
        self.root.geometry("860x600")
        self.root.resizable(False, False)

        style = ttk.Style()
        style.theme_use("clam")

        title_frame = tk.Frame(root, bg="#2c3e50", height=60)
        title_frame.pack(fill="x")
        tk.Label(
            title_frame, text="微现BOM表处理工具",
            font=("微软雅黑", 18, "bold"), bg="#2c3e50", fg="white"
        ).pack(pady=15)

        main_frame = tk.Frame(root, bg="#ecf0f1")
        main_frame.pack(fill="both", expand=True, padx=20, pady=20)

        # ========== 文件选择区 ==========
        file_frame = tk.LabelFrame(main_frame, text="选择文件", font=("微软雅黑", 10), bg="#ecf0f1", padx=10, pady=10)
        file_frame.pack(fill="x", pady=(0, 15))

        # 原始BOM文件
        tk.Label(file_frame, text="原始BOM文件：", font=("微软雅黑", 10), bg="#ecf0f1").grid(row=0, column=0, sticky="w", pady=6)
        self.bom_file_var = tk.StringVar()
        tk.Entry(file_frame, textvariable=self.bom_file_var, font=("微软雅黑", 10), width=68).grid(row=0, column=1, padx=(0, 10), pady=6)
        tk.Button(
            file_frame, text="浏览", command=self.select_bom_file, font=("微软雅黑", 10),
            bg="#3498db", fg="white", cursor="hand2", padx=12
        ).grid(row=0, column=2, pady=6)

        # 汇总池文件
        tk.Label(file_frame, text="汇总池文件：", font=("微软雅黑", 10), bg="#ecf0f1").grid(row=1, column=0, sticky="w", pady=6)
        self.pool_file_var = tk.StringVar(value=self.DEFAULT_POOL_FILE)
        tk.Entry(file_frame, textvariable=self.pool_file_var, font=("微软雅黑", 10), width=68).grid(row=1, column=1, padx=(0, 10), pady=6)
        tk.Button(
            file_frame, text="浏览", command=self.select_pool_file, font=("微软雅黑", 10),
            bg="#3498db", fg="white", cursor="hand2", padx=12
        ).grid(row=1, column=2, pady=6)

        # 处理按钮
        self.btn_process = tk.Button(
            main_frame, text="开始处理", command=self.process_file, font=("微软雅黑", 12, "bold"),
            bg="#27ae60", fg="white", cursor="hand2", padx=30, pady=10
        )
        self.btn_process.pack(pady=(0, 15))

        # 日志区
        log_frame = tk.LabelFrame(main_frame, text="处理日志", font=("微软雅黑", 10), bg="#ecf0f1", padx=10, pady=10)
        log_frame.pack(fill="both", expand=True)

        self.log_text = scrolledtext.ScrolledText(
            log_frame, font=("Consolas", 9), bg="#1f2a36", fg="#ecf0f1", height=18
        )
        self.log_text.pack(fill="both", expand=True)

        self.log_text.tag_config(LogLevel.INFO, foreground="#EAECEE")
        self.log_text.tag_config(LogLevel.DEBUG, foreground="#85C1E9")
        self.log_text.tag_config(LogLevel.WARN, foreground="#F8C471")
        self.log_text.tag_config(LogLevel.ERROR, foreground="#EC7063")
        self.log_text.tag_config(LogLevel.SUCCESS, foreground="#58D68D")

        # 状态栏
        status_frame = tk.Frame(root, bg="#34495e", height=30)
        status_frame.pack(fill="x", side="bottom")
        self.status_label = tk.Label(status_frame, text="就绪", font=("微软雅黑", 9), bg="#34495e", fg="white")
        self.status_label.pack(side="left", padx=10)

        self.app_logger = AppLogger(sink=self.gui_sink)
        self.app_logger.info("欢迎使用微现BOM表处理工具！")
        self.app_logger.info("请选择原始BOM文件和汇总池文件...")

    def gui_sink(self, level: str, message: str):
        line = AppLogger.format_line(level, message) + "\n"
        self.log_text.insert(tk.END, line, level)
        self.log_text.see(tk.END)
        self.root.update_idletasks()

    def select_bom_file(self):
        file_path = filedialog.askopenfilename(
            title="选择原始BOM文件",
            filetypes=[("Excel文件", "*.xlsx *.xls"), ("CSV文件", "*.csv"), ("所有文件", "*.*")]
        )
        if file_path:
            self.bom_file_var.set(file_path)
            self.app_logger.info(f"已选择原始BOM文件：{file_path}")

    def select_pool_file(self):
        file_path = filedialog.askopenfilename(
            title="选择汇总池文件",
            filetypes=[("Excel文件", "*.xlsx *.xls"), ("所有文件", "*.*")]
        )
        if file_path:
            self.pool_file_var.set(file_path)
            self.app_logger.info(f"已选择汇总池文件：{file_path}")

    def process_file(self):
        input_file = self.bom_file_var.get().strip()
        pool_file = self.pool_file_var.get().strip()

        if not input_file:
            messagebox.showwarning("警告", "请先选择原始BOM文件！")
            return
        if not os.path.exists(input_file):
            messagebox.showerror("错误", "原始BOM文件不存在！")
            return

        if not pool_file:
            messagebox.showwarning("警告", "请先选择汇总池文件！")
            return
        if not os.path.exists(pool_file):
            messagebox.showerror("错误", "汇总池文件不存在！")
            return

        self.btn_process.config(state="disabled")
        thread = threading.Thread(target=self._process_thread, args=(input_file, pool_file), daemon=True)
        thread.start()

    def _process_thread(self, input_file, pool_file):
        try:
            self.status_label.config(text="BOM表生成处理中...")
            generator = BOMGenerator(logger=self.app_logger)
            result = generator.generate(input_file)

            if not result.success:
                self.status_label.config(text="处理失败")
                messagebox.showerror("错误", f"BOM生成失败：\n{result.message}")
                return

            self.status_label.config(text="BOM匹配处理中...")
            matcher = BOMMatcher(
                bom_file=result.output_file,   # 用生成后的BOM文件做匹配
                pool_file=pool_file,
                config={
                    "pool_target_sheet": "26年正式购买",
                    "output_sheet": "采购表",
                    "apply_all_border": True,
                },
                logger=self.app_logger
            )
            matcher.run()

            self.status_label.config(text="处理完成")
            messagebox.showinfo("完成", f"处理成功！\n\n输出文件：\n{result.output_file}")

        except Exception as e:
            self.status_label.config(text="处理失败")
            self.app_logger.error(f"处理线程异常：{e}")
            messagebox.showerror("错误", str(e))
        finally:
            self.btn_process.config(state="normal")


def main():
    root = tk.Tk()
    ExcelProcessorGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
import sys
import requests
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox, QDialog, QVBoxLayout, QLabel
from PyQt5.QtCore import QThread, pyqtSignal, Qt
from demo import Ui_MainWindow

# ==========================================
# OneNET 平台配置参数
# ==========================================
PRODUCT_ID = "7JRHZ9cq38"
DEVICE_NAME = "test"
TOKEN = "version=2018-10-31&res=products%2F7JRHZ9cq38%2Fdevices%2Ftest&et=1803273757&method=md5&sign=Vjklk5bZHFal%2FJWc03vcHw%3D%3D"

API_QUERY_URL = "https://iot-api.heclouds.com/thingmodel/query-device-property"
API_SET_URL = "https://iot-api.heclouds.com/thingmodel/set-device-property"


# ==========================================
# 动态步骤等待窗口 (UI 组件)
# ==========================================
class WaitDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("执行中")
        self.setFixedSize(320, 120)
        # 去掉关闭按钮，防止用户在发送中途强制关闭
        self.setWindowFlags(Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint)

        self.setStyleSheet("""
            QDialog { background-color: #FFFFFF; border-radius: 12px; border: 2px solid #E2E8F0; }
            QLabel { color: #3B82F6; font-family: 'Microsoft YaHei'; font-size: 15px; font-weight: bold; }
        """)

        layout = QVBoxLayout()
        self.status_label = QLabel("正在初始化发送任务...", self)
        self.status_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.status_label)
        self.setLayout(layout)

    def update_status(self, text):
        """更新窗口中显示的当前步骤文字"""
        self.status_label.setText(text)


# ==========================================
# 后台数据获取线程 (拉取数据)
# ==========================================
class FetchDataThread(QThread):
    data_received = pyqtSignal(int, int, int)

    def run(self):
        headers = {"Authorization": TOKEN}
        params = {"product_id": PRODUCT_ID, "device_name": DEVICE_NAME}

        while True:
            try:
                response = requests.get(API_QUERY_URL, headers=headers, params=params, timeout=5)
                res_json = response.json()

                if res_json.get("code") == 0:
                    data_list = res_json.get("data", [])
                    temp_val, ph_val, light_val = 0, 0, 0

                    for item in data_list:
                        identifier = item.get("identifier")
                        raw_val = item.get("value", 0)

                        if identifier == "Temp":
                            temp_val = int(float(raw_val))
                        elif identifier == "Ph":
                            ph_val = int(float(raw_val))
                        elif identifier == "Light":
                            light_val = int(float(raw_val))

                    self.data_received.emit(temp_val, ph_val, light_val)
            except Exception:
                pass
            self.sleep(1)


# ==========================================
# 后台数据下发线程 (严格检验 STM32 应答)
# ==========================================
class PostDataThread(QThread):
    step_update = pyqtSignal(str)  # 发送当前步骤状态给等待窗口
    post_result = pyqtSignal(bool, str)  # 发送最终结果

    def __init__(self, payload):
        super().__init__()
        self.payload = payload

    def run(self):
        headers = {"Authorization": TOKEN}
        try:
            self.step_update.emit("正在将数据下发至云平台...")
            # timeout=15 意味着最多给 STM32 15秒的时间来处理并回复 ACK
            response = requests.post(API_SET_URL, headers=headers, json=self.payload, timeout=15)

            self.step_update.emit("等待 STM32 设备回传确认包...")

            if response.status_code == 200:
                res_json = response.json()

                # 【严格校验】：必须等到云端确认 STM32 已经应答 (code == 0) 才算成功
                if res_json.get("code") == 0:
                    self.post_result.emit(True, "发送并接收成功！")
                else:
                    self.post_result.emit(False, f"设备未应答或超时拒收")
            else:
                self.post_result.emit(False, f"云端网络错误: {response.status_code}")

        except requests.exceptions.ReadTimeout:
            self.post_result.emit(False, "等待 STM32 应答超时失败！")
        except Exception:
            self.post_result.emit(False, "程序网络异常，请检查连接！")


# ==========================================
# 主窗口业务逻辑
# ==========================================
class MainLogic(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        self.ui.centralwidget.setFocus()

        self.setStyleSheet(self.styleSheet() + """
            QLineEdit { color: rgba(30, 41, 59, 0.4); }
            QLineEdit:focus { color: rgba(30, 41, 59, 1.0); }
        """)

        self.limits = {
            "Tem_H": {"val": 100, "min": -100, "max": 100, "ui": self.ui.Tem_H},
            "Tem_L": {"val": -100, "min": -100, "max": 100, "ui": self.ui.Tem_L},
            "Ph_H": {"val": 140, "min": 0, "max": 140, "ui": self.ui.Ph_H},
            "Ph_L": {"val": 0, "min": 0, "max": 140, "ui": self.ui.Ph_L},
            "Lig_H": {"val": 100, "min": 0, "max": 100, "ui": self.ui.Lig_H},
            "Lig_L": {"val": 0, "min": 0, "max": 100, "ui": self.ui.Lig_L}
        }

        self.warning_state = False
        self.wait_dialog = None  # 等待窗口实例

        self.refresh_threshold_displays()
        self.ui.Send.clicked.connect(self.on_send_clicked)

        self.fetch_thread = FetchDataThread()
        self.fetch_thread.data_received.connect(self.update_realtime_data)
        self.fetch_thread.start()

    def refresh_threshold_displays(self):
        for key, config in self.limits.items():
            # 如果是 Ph 相关的阈值，除以 10 后以小数形式显示
            if key in ["Ph_H", "Ph_L"]:
                config["ui"].setText(f"{config['val'] / 10.0:.1f}")
            else:
                config["ui"].setText(str(config["val"]))

    def update_realtime_data(self, temp, ph, light):
        self.ui.Temp.display(temp)
        # 将接收到的整数 0-140，转换为 0.0-14.0 的浮点数显示在 LCD 上
        self.ui.Ph.display(f"{ph / 10.0:.1f}")
        self.ui.Light.display(light)
        self.check_alarm(temp, ph, light)

    def check_alarm(self, temp, ph, light):
        is_alarm = False
        if temp > self.limits["Tem_H"]["val"] or temp < self.limits["Tem_L"]["val"]:
            is_alarm = True
        if ph > self.limits["Ph_H"]["val"] or ph < self.limits["Ph_L"]["val"]:
            is_alarm = True
        if light > self.limits["Lig_H"]["val"] or light < self.limits["Lig_L"]["val"]:
            is_alarm = True

        if is_alarm != self.warning_state:
            self.warning_state = is_alarm
            self.update_warning_ui()

    def update_warning_ui(self):
        if self.warning_state:
            self.ui.led_warning.setStyleSheet(
                "background-color: #EF4444; border-radius: 25px; border: 3px solid #FEE2E2;")
        else:
            self.ui.led_warning.setStyleSheet(
                "background-color: #10B981; border-radius: 25px; border: 3px solid #D1FAE5;")

    def show_custom_msgbox(self, title, text, icon=QMessageBox.Information):
        msg = QMessageBox(self)
        msg.setWindowTitle(title)
        msg.setText(text)
        msg.setIcon(icon)
        msg.addButton("  确 定  ", QMessageBox.AcceptRole)

        msg.setStyleSheet("""
            QMessageBox { 
                background-color: #FFFFFF; 
            }
            QLabel { 
                color: #1E293B; 
                font-family: 'Microsoft YaHei'; 
                font-size: 15px; 
                font-weight: bold; 
                /* 核心修改1：将最小宽度强行拉大到 390px，保证再长的提示也能完整显示 */
                min-width: 390px; 
                /* 核心修改2：增加一点最小高度，让文字周围有呼吸感，显得更大气 */
                min-height: 50px; 
            }
            QPushButton {
                background-color: #3B82F6; 
                color: white;
                font-family: 'Microsoft YaHei'; 
                font-size: 15px;
                font-weight: bold;
                /* 调整为合理的黄金比例，保证按钮饱满而不臃肿 */
                padding: 8px 35px;
                border-radius: 6px;
                border: none;
            }
            QPushButton:hover { 
                background-color: #2563EB; 
            }
            QPushButton:pressed { 
                background-color: #1D4ED8; 
            }
        """)
        msg.exec_()

    def on_send_clicked(self):
        """点击发送按钮，进行极其严格且友好的数据校验"""

        # 临时字典，用于保存校验通过的值。只有所有框都校验通过，才真正写入并发送！
        temp_valid_data = {}

        for key, config in self.limits.items():
            text = config["ui"].text().strip()

            # 1. 第一道防线：判空
            if not text:
                self.show_custom_msgbox("输入为空", f"【{key}】 的值不能为空，请填写完整！", QMessageBox.Warning)
                self.refresh_threshold_displays()
                return

            # 2. 类型校验与物理极值拦截 (分开处理)
            if key in ["Ph_H", "Ph_L"]:
                # Ph 值通道：允许小数和整数
                try:
                    input_val = float(text)
                    min_f = config["min"] / 10.0
                    max_f = config["max"] / 10.0

                    # 绝对拦截：直接用原始小数进行拦截，绝不放过 14.01 或 14.09！
                    if min_f <= input_val <= max_f:
                        # 校验通过：四舍五入保留一位小数并转为整数保存
                        new_val = int(round(input_val * 10))
                        temp_valid_data[key] = new_val
                    else:
                        msg_text = f"【{key}】 数值越界！\n安全范围应在 {min_f:.1f} 到 {max_f:.1f} 之间。"
                        self.show_custom_msgbox("数值越界", msg_text, QMessageBox.Warning)
                        self.refresh_threshold_displays()
                        return
                except ValueError:
                    self.show_custom_msgbox("格式错误",
                                            f"【{key}】 格式不合法！\n此处必须输入数字（如: 7 或 7.5），不可包含字母或符号。",
                                            QMessageBox.Critical)
                    self.refresh_threshold_displays()
                    return
            else:
                # 温度/浊度通道：绝对且只能是整数
                try:
                    new_val = int(text)

                    if config["min"] <= new_val <= config["max"]:
                        temp_valid_data[key] = new_val
                    else:
                        msg_text = f"【{key}】 数值越界！\n安全范围应在 {config['min']} 到 {config['max']} 之间。"
                        self.show_custom_msgbox("数值越界", msg_text, QMessageBox.Warning)
                        self.refresh_threshold_displays()
                        return
                except ValueError:
                    if '.' in text:
                        self.show_custom_msgbox("格式错误", f"【{key}】 必须是整数！\n您输入了小数 '{text}'，请去除小数点。",
                                                QMessageBox.Critical)
                    else:
                        self.show_custom_msgbox("格式错误", f"【{key}】 包含非法字符！\n此处仅允许输入纯整数。",
                                                QMessageBox.Critical)
                    self.refresh_threshold_displays()
                    return

        # 3. 全部校验通过后，统一更新内部数据
        for key, valid_val in temp_valid_data.items():
            self.limits[key]["val"] = valid_val

        self.refresh_threshold_displays()

        # 4. 触发下发逻辑
        self.push_data_to_onenet()

    def push_data_to_onenet(self):
        payload = {
            "product_id": PRODUCT_ID,
            "device_name": DEVICE_NAME,
            "params": {
                "Tem_H": self.limits["Tem_H"]["val"],
                "Tem_L": self.limits["Tem_L"]["val"],
                "Ph_H": self.limits["Ph_H"]["val"],
                "Ph_L": self.limits["Ph_L"]["val"],
                "Lig_H": self.limits["Lig_H"]["val"],
                "Lig_L": self.limits["Lig_L"]["val"],
                "Warning": self.warning_state
            }
        }

        # 1. 弹出动态等待窗口
        self.wait_dialog = WaitDialog(self)
        self.wait_dialog.show()

        # 2. 启动严格校验的发送线程
        self.post_thread = PostDataThread(payload)
        self.post_thread.step_update.connect(self.wait_dialog.update_status)
        self.post_thread.post_result.connect(self.on_post_finished)
        self.post_thread.start()

    def on_post_finished(self, success, msg):
        # 3. 线程结束，关闭等待窗口
        if self.wait_dialog is not None:
            self.wait_dialog.accept()
            self.wait_dialog = None

        # 4. 弹出极简结果（只有：发送并接收成功 / 失败）
        if success:
            self.show_custom_msgbox("指令下发完成", msg, QMessageBox.Information)
        else:
            self.show_custom_msgbox("指令下发失败", msg, QMessageBox.Critical)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainLogic()
    window.show()
    sys.exit(app.exec_())
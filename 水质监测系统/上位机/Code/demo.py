# -*- coding: utf-8 -*-

from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(780, 420)

        # ==========================================
        # 全局现代样式表 (QSS 魔法)
        # ==========================================
        MainWindow.setStyleSheet("""
            QMainWindow {
                background-color: #F0F4F8; /* 高级灰蓝背景 */
            }
            QLabel {
                font-family: 'Microsoft YaHei', 'Segoe UI';
                font-size: 14px;
                color: #334155;
                font-weight: bold;
            }
            /* 炫酷深色数码管 */
            QLCDNumber {
                background-color: #1E293B;
                color: #10B981; 
                border: 2px solid #CBD5E1;
                border-radius: 8px;
            }
            /* 现代输入框 */
            QLineEdit {
                font-family: 'Microsoft YaHei';
                font-size: 15px;
                font-weight: bold;
                padding: 4px;
                border: 2px solid #CBD5E1;
                border-radius: 6px;
                background-color: #FFFFFF;
                color: #1E293B;
            }
            QLineEdit:focus {
                border: 2px solid #3B82F6; /* 选中时亮起蓝边 */
                background-color: #F8FAFC;
            }
            /* 动态交互按钮 */
            QPushButton {
                font-family: 'Microsoft YaHei';
                font-size: 16px;
                font-weight: bold;
                color: #FFFFFF;
                background-color: #3B82F6;
                border-radius: 10px;
            }
            QPushButton:hover {
                background-color: #2563EB;
            }
            QPushButton:pressed {
                background-color: #1D4ED8;
                padding-top: 2px; /* 按下时的下沉效果 */
            }
            /* 纯白背景卡片 */
            .CardFrame {
                background-color: #FFFFFF;
                border-radius: 12px;
            }
        """)

        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")

        # ==========================================
        # 模块 1：实时监控卡片
        # ==========================================
        self.card_top = QtWidgets.QFrame(self.centralwidget)
        self.card_top.setGeometry(QtCore.QRect(20, 20, 740, 120))
        self.card_top.setProperty("class", "CardFrame")

        # 添加高级阴影特效
        shadow1 = QtWidgets.QGraphicsDropShadowEffect()
        shadow1.setBlurRadius(15)
        shadow1.setColor(QtGui.QColor(0, 0, 0, 15))
        shadow1.setOffset(0, 4)
        self.card_top.setGraphicsEffect(shadow1)

        self.title_top = QtWidgets.QLabel(self.card_top)
        self.title_top.setGeometry(QtCore.QRect(20, 10, 150, 20))
        self.title_top.setStyleSheet("color: #94A3B8; font-size: 12px;")
        self.title_top.setObjectName("title_top")

        # --- 第 1 列：温度 ---
        self.label_7 = QtWidgets.QLabel(self.card_top)
        self.label_7.setGeometry(QtCore.QRect(25, 55, 60, 40))
        self.label_7.setObjectName("label_7")

        self.Temp = QtWidgets.QLCDNumber(self.card_top)
        self.Temp.setGeometry(QtCore.QRect(90, 50, 100, 45))
        self.Temp.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.Temp.setObjectName("Temp")

        # --- 第 2 列：Ph值 ---
        self.label_10 = QtWidgets.QLabel(self.card_top)
        self.label_10.setGeometry(QtCore.QRect(225, 55, 60, 40))
        self.label_10.setObjectName("label_10")

        self.Ph = QtWidgets.QLCDNumber(self.card_top)
        self.Ph.setGeometry(QtCore.QRect(280, 50, 100, 45))
        self.Ph.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.Ph.setObjectName("Ph")

        # --- 第 3 列：水质(光照) ---
        self.label_11 = QtWidgets.QLabel(self.card_top)
        self.label_11.setGeometry(QtCore.QRect(420, 55, 60, 40))
        self.label_11.setObjectName("label_11")

        self.Light = QtWidgets.QLCDNumber(self.card_top)
        self.Light.setGeometry(QtCore.QRect(465, 50, 100, 45))
        self.Light.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.Light.setObjectName("Light")

        # --- 第 4 列：报警指示灯 ---
        self.label_9 = QtWidgets.QLabel(self.card_top)
        self.label_9.setGeometry(QtCore.QRect(610, 20, 80, 20))
        self.label_9.setAlignment(QtCore.Qt.AlignCenter)
        self.label_9.setObjectName("label_9")

        self.led_warning = QtWidgets.QLabel(self.card_top)
        self.led_warning.setGeometry(QtCore.QRect(625, 45, 50, 50))
        # 魔法 CSS：立体的绿色发光指示灯
        self.led_warning.setStyleSheet("background-color: #10B981; border-radius: 25px; border: 3px solid #D1FAE5;")
        self.led_warning.setText("")
        self.led_warning.setObjectName("led_warning")

        # ==========================================
        # 模块 2：报警阈值控制卡片
        # ==========================================
        self.card_bottom = QtWidgets.QFrame(self.centralwidget)
        self.card_bottom.setGeometry(QtCore.QRect(20, 160, 740, 210))
        self.card_bottom.setProperty("class", "CardFrame")

        # 添加高级阴影特效
        shadow2 = QtWidgets.QGraphicsDropShadowEffect()
        shadow2.setBlurRadius(15)
        shadow2.setColor(QtGui.QColor(0, 0, 0, 15))
        shadow2.setOffset(0, 4)
        self.card_bottom.setGraphicsEffect(shadow2)

        self.title_bottom = QtWidgets.QLabel(self.card_bottom)
        self.title_bottom.setGeometry(QtCore.QRect(20, 10, 150, 20))
        self.title_bottom.setStyleSheet("color: #94A3B8; font-size: 12px;")
        self.title_bottom.setObjectName("title_bottom")

        # --- 第 1 行：高阈值 ---
        self.label = QtWidgets.QLabel(self.card_bottom)
        self.label.setGeometry(QtCore.QRect(30, 60, 60, 40))
        self.label.setObjectName("label")
        self.Tem_H = QtWidgets.QLineEdit(self.card_bottom)
        self.Tem_H.setGeometry(QtCore.QRect(90, 65, 80, 32))
        self.Tem_H.setAlignment(QtCore.Qt.AlignCenter)
        self.Tem_H.setObjectName("Tem_H")

        self.label_3 = QtWidgets.QLabel(self.card_bottom)
        self.label_3.setGeometry(QtCore.QRect(210, 60, 60, 40))
        self.label_3.setObjectName("label_3")
        self.Ph_H = QtWidgets.QLineEdit(self.card_bottom)
        self.Ph_H.setGeometry(QtCore.QRect(265, 65, 80, 32))
        self.Ph_H.setAlignment(QtCore.Qt.AlignCenter)
        self.Ph_H.setObjectName("Ph_H")

        self.label_5 = QtWidgets.QLabel(self.card_bottom)
        self.label_5.setGeometry(QtCore.QRect(390, 60, 60, 40))
        self.label_5.setObjectName("label_5")
        self.Lig_H = QtWidgets.QLineEdit(self.card_bottom)
        self.Lig_H.setGeometry(QtCore.QRect(455, 65, 80, 32))
        self.Lig_H.setAlignment(QtCore.Qt.AlignCenter)
        self.Lig_H.setObjectName("Lig_H")

        # --- 第 2 行：低阈值 ---
        self.label_2 = QtWidgets.QLabel(self.card_bottom)
        self.label_2.setGeometry(QtCore.QRect(30, 130, 60, 40))
        self.label_2.setObjectName("label_2")
        self.Tem_L = QtWidgets.QLineEdit(self.card_bottom)
        self.Tem_L.setGeometry(QtCore.QRect(90, 135, 80, 32))
        self.Tem_L.setAlignment(QtCore.Qt.AlignCenter)
        self.Tem_L.setObjectName("Tem_L")

        self.label_4 = QtWidgets.QLabel(self.card_bottom)
        self.label_4.setGeometry(QtCore.QRect(210, 130, 60, 40))
        self.label_4.setObjectName("label_4")
        self.Ph_L = QtWidgets.QLineEdit(self.card_bottom)
        self.Ph_L.setGeometry(QtCore.QRect(265, 135, 80, 32))
        self.Ph_L.setAlignment(QtCore.Qt.AlignCenter)
        self.Ph_L.setObjectName("Ph_L")

        self.label_6 = QtWidgets.QLabel(self.card_bottom)
        self.label_6.setGeometry(QtCore.QRect(390, 130, 60, 40))
        self.label_6.setObjectName("label_6")
        self.Lig_L = QtWidgets.QLineEdit(self.card_bottom)
        self.Lig_L.setGeometry(QtCore.QRect(455, 135, 80, 32))
        self.Lig_L.setAlignment(QtCore.Qt.AlignCenter)
        self.Lig_L.setObjectName("Lig_L")

        # --- 发送按钮 ---
        self.Send = QtWidgets.QPushButton(self.card_bottom)
        self.Send.setGeometry(QtCore.QRect(580, 65, 120, 102))
        self.Send.setObjectName("Send")

        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 780, 23))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "水质智能监控管理平台"))
        self.title_top.setText(_translate("MainWindow", "● 实时数据监控"))
        self.title_bottom.setText(_translate("MainWindow", "● 报警阈值配置"))
        self.Send.setText(_translate("MainWindow", "发 送 配 置"))
        self.label.setText(_translate("MainWindow", "温度-高"))
        self.label_2.setText(_translate("MainWindow", "温度-低"))
        self.label_3.setText(_translate("MainWindow", "Ph-高"))
        self.label_4.setText(_translate("MainWindow", "Ph-低"))
        self.label_5.setText(_translate("MainWindow", "水质-高"))
        self.label_6.setText(_translate("MainWindow", "水质-低"))
        self.label_7.setText(_translate("MainWindow", "水温(℃)"))
        self.label_10.setText(_translate("MainWindow", "酸碱度"))
        self.label_11.setText(_translate("MainWindow", "浊度"))
        self.label_9.setText(_translate("MainWindow", "系统警报灯"))


if __name__ == "__main__":
    import sys

    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    sys.exit(app.exec_())
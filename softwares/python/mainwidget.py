from PyQt6.QtWidgets import *
from spcawidget import SPCAWidget

class MainWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()
        
        widget = SPCAWidget()
        
        layout.addWidget(widget)
         
        self.setLayout(layout)
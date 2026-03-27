from PyQt6.QtWidgets import *
from geo_widget import GEOWidget

class MainWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()
        
        widget = GEOWidget()
        
        layout.addWidget(widget)
         
        self.setLayout(layout)
import sys
from PyQt6.QtWidgets import *
from PyQt6.QtCore import *
from mainwidget import MainWidget
  
class ApplicationWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle('Sparse PC Software')
        self.setMinimumSize(QSize(1300, 640)) 
        self.main_widget = MainWidget()
        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)
                             
def main():
    app = QApplication(sys.argv) 
    main_window = ApplicationWindow() 
    main_window.show() 
    sys.exit(app.exec()) 

if __name__ == '__main__':
    main()
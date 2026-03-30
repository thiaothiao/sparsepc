import pyqtgraph as pg
import pyqtgraph.opengl as gl

from PyQt6.QtCore import *
from PyQt6.QtWidgets import *

import pyqtgraph as pg

from spca import *
import numpy as np
from typing import List, Tuple
#from tqdm import tqdm

from pathlib import Path

from functools import partial

class SPCAWidget(QWidget):
    
    def __init__(self):
        super().__init__()

        self.layout = QHBoxLayout()
        self.layout.setContentsMargins(0,0,0,0)
        self.setLayout(self.layout)
        
        self.splitter = QSplitter()
        self.splitter.setOrientation(Qt.Orientation.Horizontal)
        self.layout.addWidget(self.splitter)
        
        self.inputs_widget = QWidget()
        inputs_layout = QVBoxLayout()
        self.inputs_widget.setLayout(inputs_layout)
        
        self.pca_action_widget = QGroupBox("Process sparse pcs")
        pca_action_layout = QVBoxLayout()
        self.pca_action_widget.setLayout(pca_action_layout)

        self.compute_sparse_candidates_button = QPushButton('Compute sparse candidates')
        pca_action_layout.addWidget(self.compute_sparse_candidates_button)
               
        self.compute_sparse_candidates_button.clicked.connect(self.on_compute_sparse_candidates)

        self.SparsityLevelSlider = QSlider(Qt.Orientation.Horizontal)
        pca_action_layout.addWidget(self.SparsityLevelSlider)

        self.clear_sparse_candidates_button = QPushButton('Clear sparse candidates')
        pca_action_layout.addWidget(self.clear_sparse_candidates_button)

        self.clear_sparse_candidates_button.clicked.connect(self.on_clear_sparse_candidates)
        
        self.pin_candidate_checkbox = QCheckBox('Pin candidate')

        pca_action_layout.addWidget(self.pin_candidate_checkbox)

        self.pin_candidate_checkbox.stateChanged.connect(self.box_changed)

        inputs_layout.addWidget(self.pca_action_widget)

        inputs_layout.addStretch(1)

        self.splitter.addWidget(self.inputs_widget)
        
        #init
        self.spca = SPCA()
        self.candidates = None

        components, colors = self.spca.run()
        m,n =components.shape

        mini = 1
        maxi = n-1
        self.SparsityLevelSlider.setRange(mini, maxi)

        self.SparsityLevelSlider.valueChanged.connect(self.update_plot)

        self.plot_widget = pg.PlotWidget()
        #plot_widget.resize(1000,600)
        #plot_widget.setWindowTitle('pyqtgraph example: Plotting')
        self.legend = self.plot_widget.addLegend()

        ax4 = self.plot_widget.getAxis('bottom')
        ax4.setLabel(text= "Time", units='days')

        # Enable antialiasing for prettier plots
        pg.setConfigOptions(antialias=True)

        # Here will be the data of the plot
        self.pc_plot_data = [None for _ in range(m)]

        self.spc_plot_data = [None]

        nbComponent, componentSize = components.shape
        x = np.arange(componentSize-1)
        self.nbComponent = nbComponent

        for i in np.arange(nbComponent):
            color = colors[i]
            component = components[i, 1:].squeeze()
            self.pc_plot_data[i] = self.plot_widget.plot(
                np.arange(component.size), component, pen=color, 
                name = f'PC {i+1}:{components[i,0]:.3f}', 
                clickable=True)
        #curves.reverse()

        def plotClicked(curve):
            for i, c in enumerate(self.pc_plot_data):
                width = 1
                color = pg.mkColor(colors[i])
                if c is curve:
                    width = 4
                    color = color.darker()
                c.setPen(color, width=width)

        for c in self.pc_plot_data:
            if c is not None:
                c.sigClicked.connect(plotClicked)
        
        self.plot_widget.setMinimumSize(QSize( 960, 640)) 
        self.splitter.addWidget(self.plot_widget)

    def on_compute_sparse_candidates(self):
        self.candidates = self.spca.computeCandidates()

    def on_clear_sparse_candidates(self):
        if self.spc_plot_data[0] is not None:
            self.plot_widget.removeItem(self.spc_plot_data[0])

        self.spc_plot_data[0] = None
        self.candidates = None

    def box_changed(self):
        return
        if self.pin_candidate_checkbox.isChecked():
            self.spc_plot_data.append(None)
        else:
            removed_element = self.spc_plot_data.pop()
            #self.state[i] = self.check_boxes[i].isChecked()
                
    def update_plot(self, iCandidate):
        # if iCandidate > 1:
        #     pass
        # self.curve.setData(data)
        if self.candidates is None:
            return
        pen = pg.mkPen(color='b', style= Qt.PenStyle.DashLine)        
        val = self.candidates[iCandidate, 0]
        component = self.candidates[iCandidate, 1:].squeeze()
        if self.spc_plot_data[0] is not None:
            self.spc_plot_data[0].setData(np.arange(component.size), component)
            self.legend.getLabel(self.spc_plot_data[0]).setText(f'SPC {1}:{val:.3f}')
        else:
            self.spc_plot_data[0] = self.plot_widget.plot(
                np.arange(component.size), component, pen=pen, 
                name = f'SPC {1}:{val:.3f}')
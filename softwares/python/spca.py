import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

import matplotlib.colors as mcolors

import sparsepc

class SPCA:
    def __init__(self):
        super().__init__()
        self.sigma = np.loadtxt(f'pitprops.csv', delimiter=';')

    def run(self):
        modelParam1 = sparsepc.linearmodel.BackwardGspcaModelParamd(13)
        modelParam2 = sparsepc.linearmodel.BackwardGspcaModelParamd(13)
        modelParam3 = sparsepc.linearmodel.BackwardGspcaModelParamd(13)
        param = sparsepc.linearmodel.BackwardGspcaParamd(
            np.array([modelParam1, modelParam2, modelParam3]))
        components = sparsepc.toMatrixd(sparsepc.linearmodel.BackwardGspcad(
            param).run(sigma = self.sigma))

        #print("\n*** Backward GSPCA variance + components ***\n")
        #print(sparsepc.toMatrixd(sparseEigenElements))

        componentColors = ['b', 'r', 'g']

        return components, componentColors
    
    def computeCandidates(self):
        modelParam = sparsepc.linearmodel.BackwardGspcaModelParamd(13)
        validatedComponents = []
        return sparsepc.toMatrixd(sparsepc.linearmodel.BackwardGspcad.computeNextComponentCandidates(
            sigma = self.sigma, param = modelParam, 
            validatedComponents = np.array(validatedComponents)))
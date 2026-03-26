import numpy as np
import sparsepc

Q = np.loadtxt(f'pitprops.csv', delimiter=';')

runForward = True
runBackward = True
runDca = True

if runForward: 
    modelParam1 = sparsepc.linearmodel.ForwardGspcaModelParamd(6)
    modelParam2 = sparsepc.linearmodel.ForwardGspcaModelParamd(2)
    modelParam3 = sparsepc.linearmodel.ForwardGspcaModelParamd(2)
    param = sparsepc.linearmodel.ForwardGspcaParamd(
        np.array([modelParam1, modelParam2, modelParam3]))
    sparseEigenElements = sparsepc.linearmodel.ForwardGspcad(param).run(sigma = Q)

    print("\n*** Forward GSPCA variance + components ***\n")
    print(sparsepc.toMatrixd(sparseEigenElements))

if runBackward: 
    modelParam1 = sparsepc.linearmodel.BackwardGspcaModelParamd(6)
    modelParam2 = sparsepc.linearmodel.BackwardGspcaModelParamd(2)
    modelParam3 = sparsepc.linearmodel.BackwardGspcaModelParamd(2)
    param = sparsepc.linearmodel.BackwardGspcaParamd(
        np.array([modelParam1, modelParam2, modelParam3]))
    sparseEigenElements = sparsepc.linearmodel.BackwardGspcad(param).run(sigma = Q)

    print("\n*** Backward GSPCA variance + components ***\n")
    print(sparsepc.toMatrixd(sparseEigenElements))

if runDca: 
    modelParam1 = sparsepc.linearmodel.DcaModelParamd(6)
    modelParam2 = sparsepc.linearmodel.DcaModelParamd(2)
    modelParam3 = sparsepc.linearmodel.DcaModelParamd(2)
    param = sparsepc.linearmodel.DcaParamd(
        np.array([modelParam1, modelParam2, modelParam3]))
    sparseEigenElements = sparsepc.linearmodel.Dcad(param).run(sigma = Q)

    print("\n*** Dca variance + components ***\n")
    print(sparsepc.toMatrixd(sparseEigenElements))

if runBackward: 
    modelParam1 = sparsepc.linearmodel.BackwardGspcaModelParamd(6)
    modelParam2 = sparsepc.linearmodel.BackwardGspcaModelParamd(2)
    modelParam3 = sparsepc.linearmodel.BackwardGspcaModelParamd(2)
    param = sparsepc.linearmodel.BackwardGspcaParamd(
        np.array([modelParam1, modelParam2, modelParam3]))
    
    validatedComponents = []
    for j in np.arange(param.nbComponents):
        candidates = sparsepc.linearmodel.BackwardGspcad.computeNextComponentCandidates(
            sigma = Q, param = param.modelParams[j], 
            validatedComponents = np.array(validatedComponents))
        
        print(sparsepc.toMatrixd(candidates))
        
        iCandidate = None
        while True:
            try:
                iCandidate = int(input("Choose one candidate\n"))
                break
            except ValueError:
                print("Invalid input! Please enter a valid integer.")

        for candidate in candidates:
            candidate.state = sparsepc.ComponentState.Unvalidated

        candidates[iCandidate].state = sparsepc.ComponentState.Validated

        validatedComponents.append(candidates[iCandidate]);

    print(sparsepc.toMatrixd(np.array(validatedComponents)))
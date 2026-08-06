---
name: centered-featurewise-matrix
description: "Use when you need to replace covariance-matrix-based reasoning or implementation with a centered featurewise matrix workflow, especially for PCA or sparse PCA code."
---

# Centered Featurewise Matrix Workflow

Use this skill when a request says to use a centered featurewise matrix instead of a covariance matrix, or when the implementation should operate on a mean-centered data matrix rather than a covariance matrix.

## Goal

Refactor the code so the primary representation is a featurewise-centered matrix, typically defined as:

```cpp
centeredX = X.rowwise() - X.colwise().mean();
```

and treat covariance only as a derived quantity when it is actually needed.

## Workflow

1. Locate the current covariance-matrix logic.
   - Find where the code computes, stores, or describes a covariance matrix.
   - Identify related comments, variable names, and docstrings.

2. Introduce or preserve the centered featurewise matrix.
   - Ensure the data is centered featurewise before downstream computations.
   - Keep the centered matrix as the main input to the solver or analysis step.

3. Reframe the explanation.
   - Prefer language such as "centered featurewise matrix" or "featurewise-centered data matrix."
   - If covariance is still mentioned, describe it as derived from the centered matrix rather than as the primary object.

4. Update affected references.
   - Rename comments and docstrings to match the centered-matrix terminology.
   - Keep numerical behavior consistent unless the task explicitly requires a mathematical change.

5. Validate the result.
   - Confirm the centered matrix is computed correctly.
   - Check that downstream code consumes it as intended.
   - Make sure the documentation and comments no longer promote covariance-matrix terminology unless it is truly relevant.

## Decision points

- If the task is about a PCA or sparse PCA solver, prefer the centered featurewise matrix as the core representation.
- If the task is purely explanatory, update wording and examples without changing numerical behavior.
- If the task requires a mathematical change, ensure the centered matrix is used before any covariance or variance computation.

## Completion checklist

- The code uses a centered featurewise matrix as the primary input.
- Comments and docs reflect the centered-matrix terminology.
- Any covariance mention is removed or clearly framed as derived from the centered matrix.
- The change is consistent with the surrounding solver code.

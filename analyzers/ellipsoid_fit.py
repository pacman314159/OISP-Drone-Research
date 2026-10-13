#!/usr/bin/env python3
"""
ellipsoid_fit.py - Rotated Ellipsoid Fitting & Magnetometer Calibration Engine
for Drone Flight Research & FC_v01 Firmware.

Formulates magnetometer calibration as a non-linear regression problem over a
full 3D rotated & offset ellipsoid using a 6-independent-variable symmetric
soft-iron matrix W and a 3-element hard-iron center offset v0:

    m_calib = B0 * W * (m_raw - v0)

Optimized via analytical gradient descent with the Adam optimizer:
    nabla_v0 = -(4/n) * W * (U^T r)
    nabla_W  =  (2/n) * (G + G^T)
    where G  =  U^T diag(r) D

Convergence / Early Stopping:
    The optimization halts when the absolute difference between consecutive
    epoch RMSE values falls below a configurable tolerance:
        |RMSE_t - RMSE_{t-1}| < tol
"""

from typing import Tuple, Dict, Any, List
import numpy as np


def fit_rotated_ellipsoid(
    mx: np.ndarray,
    my: np.ndarray,
    mz: np.ndarray,
    max_epochs: int = 500,
    lr: float = 0.02,
    tol: float = 1e-6
) -> Tuple[np.ndarray, np.ndarray, np.ndarray, Dict[str, Any]]:
    """
    Fits a full 3D rotated and offset ellipsoid to raw magnetometer readings.

    Parameters:
        mx, my, mz: 1D numpy arrays of magnetometer readings (e.g. in Gauss).
        max_epochs: Maximum gradient descent epochs (default: 500).
        lr: Learning rate for the Adam optimizer (default: 0.02).
        tol: Convergence tolerance for consecutive RMSE difference (default: 1e-6).

    Returns:
        calib_mx, calib_my, calib_mz: Calibrated 1D numpy arrays (in Gauss / target sphere).
        info: Dictionary containing:
            - 'v0': Hard-iron bias offset vector [x0, y0, z0]
            - 'W': 3x3 symmetric soft-iron correction matrix
            - 'params_9': Dict containing the 9 independent parameters
            - 'B0': Reference magnetic field magnitude (Gauss)
            - 'rmse_history': List of RMSE values across epochs
            - 'initial_rmse': Starting RMSE before optimization
            - 'final_rmse': Final converged RMSE
            - 'epochs_run': Number of epochs until convergence
            - 'converged': Boolean indicating if tolerance was reached
    """
    # 1. Prepare data matrix P (N x 3)
    P = np.column_stack((mx.astype(np.float64), my.astype(np.float64), mz.astype(np.float64)))
    n = len(P)
    if n < 9:
        raise ValueError("Insufficient points for 3D ellipsoid calibration (need at least 9 points).")

    # 2. Initial Guess
    # Center v0: empirical centroid of the point cloud
    v0 = np.mean(P, axis=0)

    # Reference radius B0: mean distance of raw samples from centroid
    diff_init = P - v0
    dists_init = np.linalg.norm(diff_init, axis=1)
    B0 = float(np.mean(dists_init))
    if B0 <= 1e-6:
        B0 = 1.0

    # Initial W: isotropic unit sphere scaling (W = (1/B0) * I)
    W = np.eye(3, dtype=np.float64) / B0

    # 3. Adam Optimizer State Initialization
    beta1 = 0.9
    beta2 = 0.999
    eps = 1e-8

    m_v0 = np.zeros(3, dtype=np.float64)
    v_v0 = np.zeros(3, dtype=np.float64)
    m_W = np.zeros((3, 3), dtype=np.float64)
    v_W = np.zeros((3, 3), dtype=np.float64)

    rmse_history: List[float] = []
    converged = False
    epochs_run = 0

    # 4. Optimization Loop
    for t in range(1, max_epochs + 1):
        epochs_run = t

        # D = P - v0 (N x 3)
        D = P - v0

        # Calibrated points: U = D * W^T = D * W (since W is symmetric)
        U = D @ W  # (N x 3)

        # Candidate radius squared: f_hat = ||u_i||^2
        f_hat = np.sum(U ** 2, axis=1)  # (N,)

        # Residual: r_i = f_hat_i - 1.0
        r = f_hat - 1.0  # (N,)

        # Root Mean Squared Error (RMSE) on residual
        mse = np.mean(r ** 2)
        rmse = float(np.sqrt(mse))
        rmse_history.append(rmse)

        # Check early stopping convergence condition
        if t > 1:
            rmse_diff = abs(rmse_history[-1] - rmse_history[-2])
            if rmse_diff < tol:
                converged = True
                break

        # -------------------------------------------------------------
        # Gradient Evaluation (Analytical Equations)
        # -------------------------------------------------------------
        # s_u = sum(r_i * u_i) = U^T r (3,)
        s_u = U.T @ r

        # grad_v0 = -(4/n) * W * s_u
        grad_v0 = (-4.0 / n) * (W @ s_u)

        # G = sum(r_i * u_i * d_i^T) = U^T (r * D) (3 x 3)
        G = U.T @ (r[:, np.newaxis] * D)

        # grad_W = (2/n) * (G + G^T) (strictly symmetric 3 x 3)
        grad_W = (2.0 / n) * (G + G.T)

        # -------------------------------------------------------------
        # Adam Parameter Updates
        # -------------------------------------------------------------
        # Update v0
        m_v0 = beta1 * m_v0 + (1.0 - beta1) * grad_v0
        v_v0 = beta2 * v_v0 + (1.0 - beta2) * (grad_v0 ** 2)
        m_v0_hat = m_v0 / (1.0 - (beta1 ** t))
        v_v0_hat = v_v0 / (1.0 - (beta2 ** t))
        v0 -= lr * m_v0_hat / (np.sqrt(v_v0_hat) + eps)

        # Update W
        m_W = beta1 * m_W + (1.0 - beta1) * grad_W
        v_W = beta2 * v_W + (1.0 - beta2) * (grad_W ** 2)
        m_W_hat = m_W / (1.0 - (beta1 ** t))
        v_W_hat = v_W / (1.0 - (beta2 ** t))
        W -= lr * m_W_hat / (np.sqrt(v_W_hat) + eps)

        # Enforce exact symmetry
        W = 0.5 * (W + W.T)

    # 5. Compute Final Calibrated Points
    # Scaled by reference field magnitude B0 so output matches Gauss units
    D_final = P - v0
    U_final = D_final @ W  # Unit sphere normalized
    P_calib = U_final * B0

    calib_mx = P_calib[:, 0]
    calib_my = P_calib[:, 1]
    calib_mz = P_calib[:, 2]

    # Explicitly package the 9 independent parameters
    params_9 = {
        # 3 Hard-Iron Offset parameters
        'v0_x': float(v0[0]),
        'v0_y': float(v0[1]),
        'v0_z': float(v0[2]),
        # 6 Soft-Iron Symmetric Matrix parameters
        'W_11': float(W[0, 0]),
        'W_22': float(W[1, 1]),
        'W_33': float(W[2, 2]),
        'W_12': float(W[0, 1]),
        'W_13': float(W[0, 2]),
        'W_23': float(W[1, 2]),
    }

    info: Dict[str, Any] = {
        'v0': v0,
        'W': W,
        'params_9': params_9,
        'B0': B0,
        'rmse_history': rmse_history,
        'initial_rmse': rmse_history[0] if rmse_history else 0.0,
        'final_rmse': rmse_history[-1] if rmse_history else 0.0,
        'epochs_run': epochs_run,
        'max_epochs': max_epochs,
        'converged': converged,
        'tol': tol
    }

    return calib_mx, calib_my, calib_mz, info

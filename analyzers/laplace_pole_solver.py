import numpy as np
import matplotlib.pyplot as plt
import control as ct

def analyze_system():
    print("General Form: Ay''(t) + By'(t) + Cy(t) + D = 0")
    try:
        # 1. User Inputs
        A = float(input("Enter A (Mass/Inertia): "))
        B = float(input("Enter B (Damping/Kd): "))
        C = float(input("Enter C (Spring/Kp): "))
        D = float(input("Enter D (Constant force/Gravity): "))
        
        # 2. Define the Characteristic Polynomial: As^2 + Bs + C
        # The poles are the roots of this polynomial
        coeffs = [A, B, C]
        poles = np.roots(coeffs)
        
        print(f"\n--- Analysis ---")
        print(f"Characteristic Equation: {A}s^2 + {B}s + {C} = 0")
        print(f"Poles found at: {poles}")
        
        # 3. Determine Stability
        if all(p.real < 0 for p in poles):
            print("Status: Asymptotically Stable (Settles to a point)")
        elif any(p.real > 0 for p in poles):
            print("Status: UNSTABLE (Will crash/diverge)")
        else:
            print("Status: Marginally Stable (Will oscillate forever)")

        # 4. Create Transfer Function for plotting
        # We use 1 as the numerator just to view the system poles
        sys = ct.TransferFunction([1], coeffs)
        
        # 5. Plotting the S-Plane
        plt.figure(figsize=(8, 6))
        ct.pzmap(sys, plot=True, title=f'S-Plane: {A}s² + {B}s + {C} = 0')
        
        # Formatting the plot
        plt.axhline(0, color='black', lw=1)
        plt.axvline(0, color='black', lw=1)
        plt.grid(True, which='both', linestyle='--', alpha=0.5)
        
        # Logic to scale the plot so we can always see the origin
        limit = max(max(abs(poles.real)), max(abs(poles.imag)), 1) * 1.5
        plt.xlim(-limit, limit)
        plt.ylim(-limit, limit)
        
        plt.show()

    except ValueError:
        print("Invalid input. Please enter numerical values.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    analyze_system()

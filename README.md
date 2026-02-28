# LSM American Option Pricer

This project implements the **Longstaff-Schwartz Least Squares Monte Carlo (LSM)** algorithm for pricing American options in C++20.

The core idea is to use regression on Monte Carlo simulated paths to estimate the continuation value at each exercise date, working backwards from maturity. This gives a provably convergent lower bound on the true option price.

> Longstaff, F.A. & Schwartz, E.S. (2001). *Valuing American Options by Simulation: A Simple Least-Squares Approach.* Review of Financial Studies, 14(1), 113–147.
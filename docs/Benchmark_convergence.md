---
title: Benchmark and Convergence
layout: default
nav_order: 3
has_children: true
---

This section documents the tools used to validate and analyse the LSM pricer.

The **Black-Scholes Pricer** provides a closed-form European option price, used as an analytical reference point. The **Convergence Analyser** runs a suite of experiments that vary key parameters — number of paths, exercise dates, basis function order, and RNG seed — comparing LSM and Finite Difference American prices against these benchmarks to assess accuracy, convergence, and stability.
---
layout: default
title: LSM Pricer UML Architecture
nav_order: 7
---

<style>
  :root {
    --basisV: #4527A0;
    --procT: #00695C;
    --payC: #BF360C;
    --engG: #1B5E20;
    --utilSl: #263238;
    --tyC: #0D47A1;
    --nmC: #4E342E;
    --ntC: #455A64;
    --abC: #B71C1C;
    --derC: #33691E;
    --bg: #f7f8fa;
    --card: #ffffff;
    --border: #d7dde2;
  }

  .lsm-wrap,
  .lsm-wrap * {
    box-sizing: border-box;
  }

  .lsm-wrap {
    max-width: 1800px;
    margin: 0 auto;
    padding: 24px;
    background: var(--bg);
    color: #111;
    font-family: "Segoe UI", Arial, sans-serif;
  }

  .lsm-layer {
    margin-bottom: 28px;
    padding: 14px;
    border: 1px solid var(--border);
    border-radius: 14px;
    background: #fcfcfd;
    box-shadow: 0 2px 8px rgba(0,0,0,0.04);
  }

  .lsm-layer-title {
    font-size: 18px;
    font-weight: 700;
    margin-bottom: 6px;
  }

  .lsm-layer-subtitle {
    font-size: 13px;
    color: #54616b;
    font-style: italic;
    margin-bottom: 16px;
  }

  .lsm-grid {
    display: grid;
    gap: 14px;
  }

  .lsm-grid.l1 { grid-template-columns: repeat(3, minmax(250px, 1fr)); }
  .lsm-grid.l2 { grid-template-columns: repeat(7, minmax(220px, 1fr)); }
  .lsm-grid.l3 { grid-template-columns: repeat(4, minmax(260px, 1fr)); }
  .lsm-grid.l4,
  .lsm-grid.l5 { grid-template-columns: 1fr; }

  .lsm-card {
    background: var(--card);
    border-radius: 10px;
    border: 2px solid;
    overflow: hidden;
  }

  .lsm-card-header {
    padding: 10px 12px;
    font-weight: 700;
    font-size: 16px;
  }

  .lsm-card-body {
    padding: 10px 12px;
    border-top: 1px solid #e9edf0;
    font-size: 13px;
    line-height: 1.5;
  }

  .lsm-card-note {
    padding: 10px 12px;
    border-top: 1px solid #e9edf0;
    font-size: 12px;
    color: var(--ntC);
    font-style: italic;
    background: #fafbfc;
  }

  .lsm-section-label {
    font-size: 11px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.04em;
    color: var(--utilSl);
    margin: 6px 0 4px;
  }

  .lsm-sig {
    font-family: Consolas, Monaco, monospace;
    font-size: 12px;
    margin: 2px 0;
    overflow-wrap: anywhere;
  }

  .lsm-type { color: var(--tyC); }
  .lsm-name { color: var(--nmC); }

  .lsm-tag {
    font-size: 11px;
    margin-left: 8px;
    font-weight: 600;
  }

  .lsm-abstract { color: var(--abC); }
  .lsm-derived { color: var(--derC); }

  .lsm-basis { border-color: var(--basisV); }
  .lsm-basis .lsm-card-header {
    background: rgba(69, 39, 160, 0.12);
    color: var(--basisV);
  }

  .lsm-proc { border-color: var(--procT); }
  .lsm-proc .lsm-card-header {
    background: rgba(0, 105, 92, 0.12);
    color: var(--procT);
  }

  .lsm-pay { border-color: var(--payC); }
  .lsm-pay .lsm-card-header {
    background: rgba(191, 54, 12, 0.12);
    color: var(--payC);
  }

  .lsm-eng { border-color: var(--engG); }
  .lsm-eng .lsm-card-header {
    background: rgba(27, 94, 32, 0.12);
    color: var(--engG);
  }

  .lsm-util { border-color: var(--utilSl); }
  .lsm-util .lsm-card-header {
    background: rgba(38, 50, 56, 0.10);
    color: var(--utilSl);
  }

  .lsm-legend {
    display: flex;
    flex-wrap: wrap;
    gap: 12px;
    margin-bottom: 24px;
    font-size: 13px;
  }

  .lsm-legend-item {
    display: flex;
    align-items: center;
    gap: 8px;
    background: white;
    border: 1px solid var(--border);
    border-radius: 999px;
    padding: 8px 12px;
  }

  .lsm-swatch {
    width: 14px;
    height: 14px;
    border-radius: 3px;
    flex: 0 0 auto;
  }

  @media (max-width: 1200px) {
    .lsm-grid.l1,
    .lsm-grid.l2,
    .lsm-grid.l3 {
      grid-template-columns: repeat(2, 1fr);
    }
  }

  @media (max-width: 700px) {
    .lsm-wrap {
      padding: 16px;
    }

    .lsm-grid.l1,
    .lsm-grid.l2,
    .lsm-grid.l3 {
      grid-template-columns: 1fr;
    }
  }
</style>

<div class="lsm-wrap">
  <div class="lsm-legend">
    <div class="lsm-legend-item"><span class="lsm-swatch" style="background:#4527A0"></span>Basis functions</div>
    <div class="lsm-legend-item"><span class="lsm-swatch" style="background:#00695C"></span>Stochastic processes</div>
    <div class="lsm-legend-item"><span class="lsm-swatch" style="background:#BF360C"></span>Payoffs</div>
    <div class="lsm-legend-item"><span class="lsm-swatch" style="background:#1B5E20"></span>Engines &amp; analysis</div>
    <div class="lsm-legend-item"><span class="lsm-swatch" style="background:#263238"></span>Utilities &amp; data</div>
  </div>

  <section class="lsm-layer">
    <div class="lsm-layer-title">Layer 1 • Abstract base classes</div>
    <div class="lsm-layer-subtitle">Pure virtual interfaces defining contracts for derived types</div>
    <div class="lsm-grid l1">
      <div class="lsm-card lsm-basis">
        <div class="lsm-card-header">BasisFunction <span class="lsm-tag lsm-abstract">«abstract»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-section-label">Pure Virtual</div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">evaluate(double x) const</span></div>
          <div class="lsm-sig"><span class="lsm-type">string</span> <span class="lsm-name">name() const</span></div>
        </div>
        <div class="lsm-card-note">Strategy pattern for regression basis</div>
      </div>

      <div class="lsm-card lsm-proc">
        <div class="lsm-card-header">StochasticProcess <span class="lsm-tag lsm-abstract">«abstract»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-section-label">Pure Virtual</div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">stepWithNormal(s, dt, z, rng) const</span></div>
          <div class="lsm-section-label">Non-Virtual</div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">step(s, dt, RNG&amp;) const</span></div>
          <div class="lsm-sig"><span class="lsm-type">vec&lt;double&gt;</span> <span class="lsm-name">simulatePath(s0, T, n, RNG&amp;)</span></div>
        </div>
        <div class="lsm-card-note">Template method pattern: <code>step()</code> calls <code>stepWithNormal()</code>.</div>
      </div>

      <div class="lsm-card lsm-pay">
        <div class="lsm-card-header">OptionPayoff <span class="lsm-tag lsm-abstract">«abstract»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-section-label">Protected</div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">K</span></div>
          <div class="lsm-section-label">Pure Virtual</div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">payoff(double S) const</span></div>
          <div class="lsm-sig"><span class="lsm-type">bool</span> <span class="lsm-name">InTheMoney(double S) const</span></div>
          <div class="lsm-sig"><span class="lsm-type">string</span> <span class="lsm-name">name() const</span></div>
          <div class="lsm-section-label">Non-Virtual</div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">strike() const</span></div>
        </div>
        <div class="lsm-card-note">Policy for payoff evaluation, ITM filtering, and strike access.</div>
      </div>
    </div>
  </section>

  <section class="lsm-layer">
    <div class="lsm-layer-title">Layer 2 • Concrete derived classes</div>
    <div class="lsm-layer-subtitle">Override abstract interfaces with concrete behaviour</div>
    <div class="lsm-grid l2">
      <div class="lsm-card lsm-basis">
        <div class="lsm-card-header">ConstantBasis <span class="lsm-tag lsm-derived">«derived»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">evaluate(double x)</span></div>
          <div class="lsm-sig"><span class="lsm-type">string</span> <span class="lsm-name">name() const</span></div>
        </div>
        <div class="lsm-card-note">Returns 1.0, the intercept term.</div>
      </div>

      <div class="lsm-card lsm-basis">
        <div class="lsm-card-header">MonomialBasis <span class="lsm-tag lsm-derived">«derived»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">int</span> <span class="lsm-name">power_</span></div>
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">MonomialBasis(int power)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">evaluate(double x)</span></div>
        </div>
        <div class="lsm-card-note">Returns x, x², x³, ...</div>
      </div>

      <div class="lsm-card lsm-basis">
        <div class="lsm-card-header">LaguerrePolynomial <span class="lsm-tag lsm-derived">«derived»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">int</span> <span class="lsm-name">order_</span></div>
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">LaguerrePolynomial(int n)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">evaluate(double x)</span></div>
        </div>
        <div class="lsm-card-note">Laguerre basis up to order n.</div>
      </div>

      <div class="lsm-card lsm-proc">
        <div class="lsm-card-header">GeometricBrownianMotion <span class="lsm-tag lsm-derived">«derived»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">r_, sigma_</span></div>
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">GBM(double r, double sigma)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">stepWithNormal(s, dt, z)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">r() const; sigma() const</span></div>
        </div>
        <div class="lsm-card-note">GBM: dS = rSdt + σSdW.</div>
      </div>

      <div class="lsm-card lsm-proc">
        <div class="lsm-card-header">JumpDiffusionProcess <span class="lsm-tag lsm-derived">«derived»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">r_, sigma_, lambda_</span></div>
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">JDP(r, sigma, lambda)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">stepWithNormal(s, dt, z, rng)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">step(s, dt, RNG&amp;) override</span></div>
        </div>
        <div class="lsm-card-note">Merton jump-diffusion model.</div>
      </div>

      <div class="lsm-card lsm-pay">
        <div class="lsm-card-header">Put_payoff <span class="lsm-tag lsm-derived">«derived»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">Put_payoff(double K)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">payoff(double S) const</span></div>
          <div class="lsm-sig"><span class="lsm-type">bool</span> <span class="lsm-name">InTheMoney(double S) const</span></div>
        </div>
        <div class="lsm-card-note">max(K - S, 0); ITM if S &lt; K.</div>
      </div>

      <div class="lsm-card lsm-pay">
        <div class="lsm-card-header">Call_payoff <span class="lsm-tag lsm-derived">«derived»</span></div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">Call_payoff(double K)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">payoff(double S) const</span></div>
          <div class="lsm-sig"><span class="lsm-type">bool</span> <span class="lsm-name">InTheMoney(double S) const</span></div>
        </div>
        <div class="lsm-card-note">max(S - K, 0); ITM if S &gt; K.</div>
      </div>
    </div>
  </section>

  <section class="lsm-layer">
    <div class="lsm-layer-title">Layer 3 • Support &amp; utility</div>
    <div class="lsm-layer-subtitle">RNG, basis ownership, regression, config, data, and reference pricers</div>
    <div class="lsm-grid l3">
      <div class="lsm-card lsm-util">
        <div class="lsm-card-header">RNG</div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">mt19937_64</span> <span class="lsm-name">engine</span></div>
          <div class="lsm-sig"><span class="lsm-type">normal_dist</span> <span class="lsm-name">norm_dist</span></div>
          <div class="lsm-sig"><span class="lsm-type">uniform_dist</span> <span class="lsm-name">uniform_dist</span></div>
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">RNG(unsigned seed)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">normal()</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">uniform01()</span></div>
        </div>
        <div class="lsm-card-note">Encapsulates a seed-reproducible PRNG.</div>
      </div>

      <div class="lsm-card lsm-util">
        <div class="lsm-card-header">BasisSet</div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">vec&lt;unique_ptr&lt;BF&gt;&gt;</span> <span class="lsm-name">basis</span></div>
          <div class="lsm-sig"><span class="lsm-type">vec&lt;BF*&gt;</span> <span class="lsm-name">basisPtrs() const</span></div>
          <div class="lsm-sig"><span class="lsm-type">void</span> <span class="lsm-name">makeLaguerreSet(int n)</span></div>
          <div class="lsm-sig"><span class="lsm-type">void</span> <span class="lsm-name">makeMonomialSet(int n)</span></div>
        </div>
        <div class="lsm-card-note">Owns basis functions via <code>unique_ptr</code> with factory helpers.</div>
      </div>

      <div class="lsm-card lsm-util">
        <div class="lsm-card-header">OLS (free functions — lsm::engine)</div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">MatrixXd</span> <span class="lsm-name">buildDesignMatrix(S_t, itm, basis, strike)</span></div>
          <div class="lsm-sig"><span class="lsm-type">vec&lt;double&gt;</span> <span class="lsm-name">buildYVector(cfs, itm, df)</span></div>
          <div class="lsm-sig"><span class="lsm-type">vec&lt;bool&gt;</span> <span class="lsm-name">getITMVector(S_t, payoff)</span></div>
          <div class="lsm-sig"><span class="lsm-type">vec&lt;double&gt;</span> <span class="lsm-name">Ols_regression(paths, t, cfs, itm, df, basis, strike)</span></div>
        </div>
        <div class="lsm-card-note">ITM filter → design matrix → QR solve → continuation coefficients.</div>
      </div>

      <div class="lsm-card lsm-util">
        <div class="lsm-card-header">LSMConfig / PathData / SimulationResult / BSPricer / FDPricer</div>
        <div class="lsm-card-body">
          <div class="lsm-sig"><span class="lsm-type">LSMConfig</span> <span class="lsm-name">numPaths, useAntithetic, numExerciseDates, maturity, riskFreeRate, rngSeed</span></div>
          <div class="lsm-sig"><span class="lsm-type">PathData</span> <span class="lsm-name">paths, cashFlows, numPaths, numTimeSteps</span></div>
          <div class="lsm-sig"><span class="lsm-type">SimulationResult</span> <span class="lsm-name">optionValue, standardError, europeanValue, earlyExercisePremium</span></div>
          <div class="lsm-sig"><span class="lsm-type">BSPricer</span> <span class="lsm-name">Black-Scholes European price</span></div>
          <div class="lsm-sig"><span class="lsm-type">FDPricer</span> <span class="lsm-name">Crank-Nicolson / implicit FD American pricer</span></div>
        </div>
        <div class="lsm-card-note">Configuration, data containers, outputs, and reference pricing tools.</div>
      </div>
    </div>
  </section>

  <section class="lsm-layer">
    <div class="lsm-layer-title">Layer 4 • Pricing engine</div>
    <div class="lsm-layer-subtitle">Main Longstaff-Schwartz orchestrator</div>
    <div class="lsm-grid l4">
      <div class="lsm-card lsm-eng">
        <div class="lsm-card-header">LSMPricer</div>
        <div class="lsm-card-body">
          <div class="lsm-section-label">Private Members</div>
          <div class="lsm-sig"><span class="lsm-type">const StochasticProcess&amp;</span> <span class="lsm-name">process</span></div>
          <div class="lsm-sig"><span class="lsm-type">const OptionPayoff&amp;</span> <span class="lsm-name">payoff</span></div>
          <div class="lsm-sig"><span class="lsm-type">const BasisSet&amp;</span> <span class="lsm-name">basis</span></div>
          <div class="lsm-sig"><span class="lsm-type">LSMConfig</span> <span class="lsm-name">config</span></div>

          <div class="lsm-section-label">Public</div>
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">LSMPricer(const StochasticProcess&amp;, const OptionPayoff&amp;, const BasisSet&amp;, const LSMConfig&amp;)</span></div>
          <div class="lsm-sig"><span class="lsm-type">SimulationResult</span> <span class="lsm-name">price(double S0)</span></div>
          <div class="lsm-sig"><span class="lsm-type">pair&lt;SimulationResult, PathData&gt;</span> <span class="lsm-name">priceWithData(double S0)</span></div>

          <div class="lsm-section-label">Private Algorithm Steps</div>
          <div class="lsm-sig"><span class="lsm-type">vec&lt;double&gt;</span> <span class="lsm-name">backwardInduction(PathData&amp;) const</span></div>
          <div class="lsm-sig"><span class="lsm-type">SimulationResult</span> <span class="lsm-name">computeOptionValue(vec&lt;double&gt;&amp;, double, int, int) const</span></div>
        </div>
        <div class="lsm-card-note">price() → simulatePaths() → backwardInduction() → OLS regression → computeOptionValue()</div>
      </div>
    </div>
  </section>

  <section class="lsm-layer">
    <div class="lsm-layer-title">Layer 5 • Analysis &amp; convergence tools</div>
    <div class="lsm-layer-subtitle">Benchmarks and convergence experiments</div>
    <div class="lsm-grid l5">
      <div class="lsm-card lsm-eng">
        <div class="lsm-card-header">ConvergenceAnalyser</div>
        <div class="lsm-card-body">
          <div class="lsm-section-label">Private</div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">S0, r, sigma, K, T</span></div>
          <div class="lsm-sig"><span class="lsm-type">function</span> <span class="lsm-name">sdeFactory(double r, double sigma) -&gt; unique_ptr&lt;StochasticProcess&gt;</span></div>
          <div class="lsm-sig"><span class="lsm-type">OptionPayoff&amp;</span> <span class="lsm-name">payoffType</span></div>
          <div class="lsm-sig"><span class="lsm-type">BasisSet&amp;</span> <span class="lsm-name">basisType</span></div>
          <div class="lsm-sig"><span class="lsm-type">function</span> <span class="lsm-name">basisFactory(BasisSet&amp;, int order) -&gt; void</span></div>
          <div class="lsm-sig"><span class="lsm-type">int</span> <span class="lsm-name">order, fixedPathCount, fixedNumDates</span></div>

          <div class="lsm-section-label">Public</div>
          <div class="lsm-sig"><span class="lsm-type"></span> <span class="lsm-name">ConvergenceAnalyser(...)</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">getBSPrice(); getFDPrice()</span></div>
          <div class="lsm-sig"><span class="lsm-type">double</span> <span class="lsm-name">getLSMPrice(seed, numExerciseDates, order, numPaths)</span></div>
          <div class="lsm-sig"><span class="lsm-type">SimulationResult</span> <span class="lsm-name">getLSMResult(seed, numExerciseDates, order, numPaths, antithetic)</span></div>
          <div class="lsm-sig"><span class="lsm-type">void</span> <span class="lsm-name">runBenchmark(); runSeedStability()</span></div>
          <div class="lsm-sig"><span class="lsm-type">void</span> <span class="lsm-name">runPathConvergence(pathCounts); runDatesConvergence(exerciseDatesList)</span></div>
          <div class="lsm-sig"><span class="lsm-type">void</span> <span class="lsm-name">runOrderConvergence(orders); runFDConvergence(timeCounts, stockSteps)</span></div>
        </div>
        <div class="lsm-card-note">Constructs LSMPricer variants via factories, benchmarks against BS and FD, and writes CSV output to <code>csv_output/</code>.</div>
      </div>
    </div>
  </section>
</div>

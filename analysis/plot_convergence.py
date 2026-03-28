import pandas as pd
import matplotlib.pyplot as plt
import os

def generate_convergence_report():
    input_dir = 'csv_output'
    output_dir = 'analysis'

    # make the folder if it doesn't already exists
    os.makedirs(output_dir, exist_ok=True)
    for f in os.listdir(output_dir):
        if f.endswith('.png'):
            os.remove(os.path.join(output_dir, f))

    plot_configs = [
        {
            'name': 'path_convergence.csv',
            'column': 'Number of Paths',
            'title': 'LSM Convergence: Path Count',
            'use_log': True,
            'filename': 'path_convergence.png'
        },
        {
            'name': 'order_convergence.csv',
            'column': 'Order of Basis',
            'title': 'LSM Convergence: Basis Order',
            'use_log': False,
            'filename': 'order_convergence.png'
        },
        {
            'name': 'exercise_dates_convergence.csv',
            'column': 'Number of Exercise Dates',
            'title': 'LSM Convergence: Exercise Dates',
            'use_log': False,
            'filename': 'exercise_dates_convergence.png'
        }
    ]

    option_types = ['call', 'put']
    basis_types  = ['laguerre', 'monomial']
    markers      = {'laguerre': 'o', 'monomial': 'x'}

    for config in plot_configs:
        fig_price, ax_price = plt.subplots(figsize=(10, 6))
        fig_time,  ax_time  = plt.subplots(figsize=(10, 6))

        for opt in option_types:
            for basis in basis_types:
                path = os.path.join(input_dir, f"{opt}_{basis}_{config['name']}")
                if not os.path.exists(path):
                    print(f"Warning: Missing {path}. Skipping...")
                    continue

                df = pd.read_csv(path)
                label = f"{basis.capitalize()} ({opt})"

                ax_price.plot(df[config['column']], df['Error'],
                              marker=markers[basis], label=label, linewidth=1.5)
                ax_time.plot(df[config['column']], df['Time(ms)'],
                             marker=markers[basis], label=label, linewidth=1.5)

        for ax, ylabel, suffix in [
            (ax_price, 'Absolute Error', config['filename']),
            (ax_time,  'Time (ms)',    config['filename'].replace('.png', '_timing.png'))
        ]:
            if config['use_log']:
                ax.set_xscale('log')
            ax.set_title(config['title'])
            ax.set_xlabel(config['column'])
            ax.set_ylabel(ylabel)
            ax.legend()
            ax.grid(True, which="both", linestyle='--', alpha=0.5)
            save_path = os.path.join(output_dir, suffix)
            ax.figure.tight_layout()
            ax.figure.savefig(save_path, dpi=300)
            plt.close(ax.figure)
            print(f"Generated: {save_path}")


if __name__ == "__main__":
    generate_convergence_report()
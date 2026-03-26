import pandas as pd
import matplotlib.pyplot as plt
import os

def generate_convergence_report():
    input_dir = 'csv_output'
    output_dir = 'analysis'

    # make the folder if it doesn't already exists
    os.makedirs(output_dir, exist_ok=True)

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

    # iterate through the variables
    for config in plot_configs:
        # construct paths for both basis types
        lag_path = os.path.join(input_dir, f"laguerre_{config['name']}")
        mon_path = os.path.join(input_dir, f"monomial_{config['name']}")

        if not os.path.exists(lag_path) or not os.path.exists(mon_path):
            print(f"Warning: Missing data for {config['title']}. Skipping...")
            continue

        # load data
        df_lag = pd.read_csv(lag_path)
        df_mon = pd.read_csv(mon_path)

        # create plot
        plt.figure(figsize=(10, 6))
        
        # plot both basis types on one graph
        plt.plot(df_lag[config['column']], df_lag['LSMPrice'], 
                 marker='o', label='Laguerre', linewidth=1.5)
        plt.plot(df_mon[config['column']], df_mon['LSMPrice'], 
                 marker='s', label='Monomial', linewidth=1.5, alpha=0.7)
        

        # formatting
        if config['use_log']:
            plt.xscale('log')
        
        plt.title(config['title'])
        plt.xlabel(config['column'])
        plt.ylabel('Option Price')
        plt.legend()
        plt.grid(True, which="both", linestyle='--', alpha=0.5)
        plt.tight_layout()

        # save to the analysis folder
        save_path = os.path.join(output_dir, config['filename'])
        plt.savefig(save_path, dpi=300)
        plt.close() 
        
        print(f"Generated: {save_path}")

if __name__ == "__main__":
    generate_convergence_report()
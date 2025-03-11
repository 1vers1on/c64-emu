import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('build/floppy.log', header=None)

fig, axes = plt.subplots(3, 1, figsize=(8, 6), sharex=True)

axes[0].plot(data.index, data[0], label='Data', color='r', drawstyle='steps-post')
axes[0].set_ylabel('Value')
axes[0].set_title('Data')
axes[0].grid(True)
axes[0].legend()

axes[1].plot(data.index, data[1], label='Clock', color='g', drawstyle='steps-post')
axes[1].set_ylabel('Value')
axes[1].set_title('Clock')
axes[1].grid(True)
axes[1].legend()

axes[2].plot(data.index, data[2], label='Attention', color='b', drawstyle='steps-post')
axes[2].set_xlabel('Tick')
axes[2].set_ylabel('Attention')
axes[2].set_title('Attention')
axes[2].grid(True)
axes[2].legend()

plt.tight_layout()
plt.savefig('line_plots.png')

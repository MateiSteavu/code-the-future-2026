import numpy as np
import pyvista as pv
import time
import pandas as pd

current_actor = None
last_point_count = -1  # track changes by count

plotter = pv.Plotter()
plotter.set_background('grey')
plotter.show(interactive_update=True)

while True:
    try:
        data = pd.read_csv('test.csv', header=None).dropna().values

        if data.size == 0:
            plotter.update()
            time.sleep(0.5)
            continue

        if data.ndim == 1:
            data = data.reshape(1, -1)

        # Re-render whenever file content changes
        if len(data) != last_point_count:
            last_point_count = len(data)

            if current_actor is not None:
                plotter.remove_actor(current_actor)

            points = data[:, :3].astype(float)
            cloud = pv.PolyData(points)
            glyphs = cloud.glyph(scale=False, geom=pv.Cube(), factor=2.0)
            current_actor = plotter.add_mesh(
                glyphs, color='red', show_edges=True, edge_color='black'
            )
            plotter.reset_camera()

    except Exception as e:
        print(f"error: {e}")

    plotter.update()
    time.sleep(0.5)
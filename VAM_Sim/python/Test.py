# #dots
# # import numpy as np
# # import matplotlib.pyplot as plt

# # threshold = 5.0

# # with open('test.txt', 'r') as f:
# #     arrays = [np.array(list(map(float, line.split()))) for line in f]

# # points = [arr for arr in arrays if arr[3] > threshold]
# # points = np.array(points)

# # ax = plt.figure().add_subplot(projection='3d')
# # ax.scatter(points[:, 0], points[:, 1], points[:, 2])
# # plt.show()

# # import numpy as np
# # import matplotlib.pyplot as plt

# #cubes
# # threshold = 5.0
# # sizeofcube=5.0
# # data = np.loadtxt('test.txt')
# # filtered = data[data[:, 3] > threshold]
 
# # fig = plt.figure()
# # ax = fig.add_subplot(projection='3d')

# # for row in filtered:
# #     ax.bar3d(row[0], row[1], row[2], sizeofcube, sizeofcube, sizeofcube, alpha=0.7)

# # plt.show()

# #pyvista


# #one image showcase
# # import numpy as np
# # import pyvista as pv

# # threshold = 5.0

# # data = np.loadtxt('test.txt')
# # filtered = data[data[:, 3] > threshold]

# # # create a point cloud from float coordinates
# # points = filtered[:, :3]
# # cloud = pv.PolyData(points)

 

# # # convert points to cubes
# # glyphs = cloud.glyph(scale=False, geom=pv.Cube())

# # plotter = pv.Plotter()
# # plotter.add_mesh(glyphs,   show_edges=True)
# # plotter.show()





# # working, waits for data
import numpy as np
import pyvista as pv
import time
import pandas as pd

seen_count = 0
BATCH_SIZE = 50

plotter = pv.Plotter()
plotter.set_background('grey')
plotter.show(interactive_update=True)

while True:
    try:
        data = pd.read_csv('test.csv', header=None).dropna().values
        if data.size == 0:
            continue
        if data.ndim == 1:
            data = data.reshape(1, -1)
        
        if len(data) > seen_count:
            new_points = data[seen_count:seen_count + BATCH_SIZE, :3]
            cloud = pv.PolyData(new_points.astype(float))
            glyphs = cloud.glyph(scale=False, geom=pv.Cube(), factor=2.0)
            plotter.add_mesh(glyphs, color='red', show_edges=True, edge_color='black')
            seen_count += len(new_points)
            plotter.reset_camera()

    except Exception as e:
        print(f"error: {e}")
    
    plotter.update()
    time.sleep(0.1)


# # working, doesnt need data as it generates it
# import random
# import threading
# import time
# import numpy as np
# import pyvista as pv
# import pandas as pd

 
# def generate():
#     num_lines = 25
#     size = 100
#     with open('test.csv', 'w') as f:
#         for s in range(num_lines):
#             x = round(random.uniform(0, size), 1)
#             y = round(random.uniform(0, size), 1)
#             z = round(random.uniform(0, size), 1)
#             f.write(f"{x},{y},{z}\n")
#             f.flush()
#             time.sleep(1)

 
# seen_count = 0
# BATCH_SIZE = 50

# thread = threading.Thread(target=generate)
# thread.daemon = True
# thread.start()

# plotter = pv.Plotter()
# plotter.set_background('grey')
# plotter.show(interactive_update=True)

# while True:
#     try:
#         data = pd.read_csv('test.csv', header=None).dropna().values
#         if data.size == 0:
#             continue
#         if data.ndim == 1:
#             data = data.reshape(1, -1)
#         if len(data) > seen_count:
#             new_points = data[seen_count:seen_count + BATCH_SIZE, :3]
#             cloud = pv.PolyData(new_points.astype(float))
#             glyphs = cloud.glyph(scale=False, geom=pv.Cube(), factor=2.0)
#             plotter.add_mesh(glyphs, color='red', show_edges=True, edge_color='black')
#             seen_count += len(new_points)
#             plotter.reset_camera()
#     except Exception as e:
#         print(f"error: {e}")
    
#     plotter.update()
#     time.sleep(0.1)
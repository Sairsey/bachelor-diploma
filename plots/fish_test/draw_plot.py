import matplotlib.pyplot as plt

direct_fish = []
direct_fps = []
direct_cpu_time = []
direct_gpu_time = []
with open("direct_perf.txt", "r") as f:
    for line in f:
        fish, eng, cpu, gpu = [float(t) for t in line.split(', ')]
        direct_fish.append(fish)
        direct_fps.append(1000000000.0 / eng)
        direct_cpu_time.append(cpu / 1000000.0)
        direct_gpu_time.append(gpu / 1000.0)
 
indirect_fish = []
indirect_fps = []
indirect_cpu_time = []
indirect_gpu_time = []
with open("indirect_perf.txt", "r") as f:
    for line in f:
        fish, eng, cpu, gpu = [float(t) for t in line.split(', ')]
        indirect_fish.append(fish)
        indirect_fps.append(1000000000.0 / eng)
        indirect_cpu_time.append(cpu / 1000000.0)
        indirect_gpu_time.append(gpu / 1000.0)
                
plt.plot(direct_fish, direct_fps, label="direct")
plt.plot(indirect_fish, indirect_fps, label="indirect")
plt.legend()
plt.title("No Culling FPS")
plt.xlabel("Fish amount")
plt.ylabel("FPS")
plt.figure()
                
plt.plot(direct_fish, direct_cpu_time, label="direct cpu time")
plt.plot(indirect_fish, indirect_cpu_time, label="indirect cpu time")
plt.legend()
plt.title("No Culling CPU Frame time")
plt.xlabel("Fish amount")
plt.ylabel("mS")
plt.figure()

plt.plot(direct_fish, direct_gpu_time, label="direct gpu time")
plt.plot(indirect_fish, indirect_gpu_time, label="indirect gpu time")
plt.legend()
plt.title("No Culling GPU Frame time")
plt.xlabel("Fish amount")
plt.ylabel("mS")
plt.figure()

direct_fish = []
direct_fps = []
direct_cpu_time = []
direct_gpu_time = []
with open("direct_cull_perf.txt", "r") as f:
    for line in f:
        fish, eng, cpu, gpu = [float(t) for t in line.split(', ')]
        direct_fish.append(fish)
        direct_fps.append(1000000000.0 / eng)
        direct_cpu_time.append(cpu / 1000000.0)
        direct_gpu_time.append(gpu / 1000.0)
 
indirect_fish = []
indirect_fps = []
indirect_cpu_time = []
indirect_gpu_time = []
with open("indirect_cull_perf.txt", "r") as f:
    for line in f:
        fish, eng, cpu, gpu = [float(t) for t in line.split(', ')]
        indirect_fish.append(fish)
        indirect_fps.append(1000000000.0 / eng)
        indirect_cpu_time.append(cpu / 1000000.0)
        indirect_gpu_time.append(gpu / 1000.0)
                
plt.plot(direct_fish, direct_fps, label="direct")
plt.plot(indirect_fish, indirect_fps, label="indirect")
plt.legend()
plt.title("Frustum Culling FPS")
plt.xlabel("Fish amount")
plt.ylabel("FPS")
plt.figure()
                
plt.plot(direct_fish, direct_cpu_time, label="direct cpu time")
plt.plot(indirect_fish, indirect_cpu_time, label="indirect cpu time")
plt.legend()
plt.title("Frustum Culling CPU Frame time")
plt.xlabel("Fish amount")
plt.ylabel("mS")
plt.figure()

plt.plot(direct_fish, direct_gpu_time, label="direct gpu time")
plt.plot(indirect_fish, indirect_gpu_time, label="indirect gpu time")
plt.legend()
plt.title("Frustum Culling GPU Frame time")
plt.xlabel("Fish amount")
plt.ylabel("mS")
plt.figure()

plt.show()


import numpy as np
import matplotlib.pyplot as plt

# Открываем файл для чтения
name = "/home/nsk/dev/test/sdrLessons/build/txdata.pcm"

data = []
imag = []
real = []
count = []
counter = 0
with open(name, "rb") as f:
    index = 0
    while (byte := f.read(2)):
        if(index %2 == 0):
            real.append(int.from_bytes(byte, byteorder='little', signed=True))
            counter += 1
            count.append(counter)
        else:
            imag.append(int.from_bytes(byte, byteorder='little', signed=True))
        index += 1
        
# Инициализируем список для хранения данных


# fig, axs = plt.subplots(2, 1, layout='constrained')


# plt.figure(1)
# # axs[1].plot(count, np.abs(data),  color='grey')  # Используем scatter для диаграммы созвездия
# plt.plot(count,(imag),color='red')  # Используем scatter для диаграммы созвездия
# plt.plot(count,(real), color='blue')  # Используем scatter для диаграммы созвездия
# plt.show()

count = 0
file = open("out.txt", "w")
file.write("std::vector<std::complex<double>> rx_samples = {")
for i in range(2414, 20600):
    file.write("{")
    file.write(str(real[i]))
    file.write(", ")
    file.write(str(imag[i]))
    file.write("},")
    count += 1
    if(count % 20 == 0):
        file.write("\n")
file.write("\n")
file.write("};")
file.close()
print(count)

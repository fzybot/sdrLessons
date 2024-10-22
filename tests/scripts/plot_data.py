import numpy as np
import matplotlib.pyplot as plt

# Открываем файл для чтения
with open('/home/novosibirsk/dev/sdrLessons/build/2_rx_signal.txt', 'r') as file:
    # Читаем все строки из файла
    lines = file.readlines()

# Инициализируем список для хранения данных
data = []

# Обрабатываем каждую строку
for line in lines:
    line = line.strip()  # Убираем лишние пробелы
    if line:  # Пропускаем пустые строки
        try:
            # Разделяем строку по запятой и преобразуем в числа
            numbers = list(map(float, [value.strip() for value in line.split(',')]))
            # Создаём комплексное число (I + jQ)
            complex_number = complex(numbers[0], numbers[1])
            data.append(complex_number)
        except ValueError:
            print(f"Ошибка преобразования строки: {line}")

# Преобразуем список в массив NumPy
rx_nothreads = np.array(data)

# Вывод загруженных данных
#print(rx_nothreads)
plt.figure(1)
plt.plot(np.abs(data))  # Используем scatter для диаграммы созвездия
plt.xlabel('I (Real)')
plt.ylabel('Q (Imaginary)')
plt.title('QAM Constellation Diagram')
plt.grid(True)
plt.show()
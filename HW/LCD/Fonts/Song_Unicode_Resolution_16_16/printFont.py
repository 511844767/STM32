# 打开文件
with open(r"E:\Project\EMProject\STM32_STD\HW\LCD\Fonts\Song_Unicode_Resolution_16_16\ENG\eng.bin", "rb") as f:
    f.seek(0X61 * 32)
    data = f.read(32)


# 打印"a"的字模
for i in range(32 * 8):
    if(data[i // 8] & (0B10000000 >> (i % 8))):
        print("*", end="")
    else:
        print(" ", end="")
    if(i % 16 == 0):
        print("")
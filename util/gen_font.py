import pygame as pg

square = 8
width = 16
height = 8

def encode_character(image, x, y):
    bs = []
    for py in range(square):
        p = 0x80
        b = 0
        for px in range(square):
            if image.get_at((x * square + px, y * square + py)) == (255, 255, 255):
                b = b | p
                print(f"0x{b:02x}, {p:08b}")

            p = p >> 1
        bs.append(b)

    return bs

def main():
    bmp = pg.image.load("util/font-template.png")

    bs = []

    for y in range(height):
        for x in range(width):
            i = y * width + x
            print("##########")
            print(f"Char {chr(i)} ({i})")
            char = encode_character(bmp, x, y)
            print(char)
            bs += char

    with open("font.h", "w") as file:
        file.write(f"CE_u64 font_length = {len(bs) // 8};\n")
        file.write("CE_u8 font[] = {\n")
        for i in range(0, len(bs), 8):
            file.write("    ")
            for b in bs[i:i+7]:
                file.write(f"0x{b:02x}, ")
            file.write(f"0x{bs[i+7]:02x},\n")
        file.write("};\n")

if __name__ == '__main__':
    pg.init()
    main()
    pg.quit()

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
    screen = pg.display.set_mode((width * square * 3, height * square * 3))
    font = pg.font.Font("res/fonts/PublicPixel.ttf", 7)

    bmp = pg.Surface((width * square, height * square))

    for y in range(height):
        for x in range(width):
            i = y * width + x
            if i < 32:
                continue
            text = font.render(chr(i), False, (255, 255, 255))
            bmp.blit(text, (x * square, y * square))

    pg.image.save(bmp, "util/font-template.png")

    running = True
    while running:
        for e in pg.event.get():
            if e.type == pg.QUIT:
                running = False

        screen.blit(pg.transform.scale_by(bmp, 3), (0, 0))
        pg.display.flip()

if __name__ == '__main__':
    pg.init()
    main()
    pg.quit()

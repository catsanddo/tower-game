import pygame as pg
import json


class Portal:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.toX = 0
        self.toY = 0
        self.toMap = input('To Map? ')

    def to_dict(self):
        result = {
                'x': self.x,
                'y': self.y,
                'toX': self.toX,
                'toY': self.toY,
                'toMap': self.toMap
                }
        return result


def load_map(path):
    data = ''
    with open(path, 'r') as file:
        data = file.read()
    root = json.loads(data)

    map_surface = pg.Surface((root['width'] * 16, root['height'] * 16))
    tiles = pg.image.load(root['tileSheet'])

    for y in range(root['height']):
        for x in range(root['width']):
            index = root['tileMap'][y*root['width']+x] - 1
            if index > 2000:
                index -= 1<<31
            src = pg.Rect((index % (tiles.get_width() // 16) * 16, index // (tiles.get_width() // 16) * 16), (16, 16))
            map_surface.blit(tiles, (x*16, y*16), src)

    return (map_surface, root)


def save_map(path, root, portals):
    root['portals'] = []
    for portal in portals:
        root['portals'].append(portal.to_dict())

    data = json.dumps(root)
    with open(path, 'w') as file:
        file.write(data)


def main():
    path = input('Map? ')
    map_surface, root = load_map(path)
    location = None
    screen = pg.display.set_mode(map_surface.get_size())
    portals = []

    select = False
    running = True
    while running:
        for e in pg.event.get():
            if e.type == pg.QUIT:
                running = False
        m_pos = pg.mouse.get_pos()
        m_but = pg.mouse.get_pressed()

        if m_but[0] and not select:
            x = m_pos[0] - (m_pos[0] % 16)
            y = m_pos[1] - (m_pos[1] % 16)
            b = pg.Surface((16, 16))
            b.fill((255, 0, 0))
            b.set_alpha(100)
            map_surface.blit(b, (x, y))

            portals.append(Portal(x // 16, y // 16))
            select = True
            location = load_map(portals[-1].toMap)[0]
            pg.display.quit()
            screen = pg.display.set_mode(location.get_size())
        if select:
            if m_but[2]:
                x = m_pos[0] - (m_pos[0] % 16)
                y = m_pos[1] - (m_pos[1] % 16)

                portals[-1].toX = x // 16
                portals[-1].toY = y // 16
                select = False

                pg.display.quit()
                screen = pg.display.set_mode(map_surface.get_size())

        if select:
            screen.blit(location, (0, 0))
        else:
            screen.blit(map_surface, (0, 0))
        pg.display.flip()

    if input('Overwrite? ') == 'yes':
        save_map(path, root, portals)
    else:
        save_map('map.json', root, portals)


if __name__ == '__main__':
    pg.init()
    main()
    pg.quit()

import random

import blipper

def dump_to_files(name, cmd):
    with open('dumps/%s.bin' % name, 'wb') as f:
        f.write(bytes(cmd))
    with open('dumps/%s.txt' % name, 'wb') as f:
        f.write('%s:\n' % name)
        f.write(" ".join(hex(ord(b)) for b in bytes(cmd)))
        f.write('\n\n')


def dump():
    insufficient_funds = blipper.InsufficientFunds.from_balance(1337)
    dump_to_files('insufficient_funds', insufficient_funds)

    new_balance = blipper.NewBalance.from_new_balance(1337)
    dump_to_files('new_balance', new_balance)

    dump_to_files('ping', blipper.Ping())
    dump_to_files('pong', blipper.Pong())
    dump_to_files('ok', blipper.Ok())
    dump_to_files('fail', blipper.Fail())
    dump_to_files('invalid_card_id', blipper.InvalidCardId())
    dump_to_files('next_shift', blipper.NextShift.from_text(u'fm 2012-04-23'))

    display = 0
    rows_on = [[1] * 128] * 64
    redraw_pixels_all_on = blipper.RedrawPixels.from_matrix(display, rows_on)
    dump_to_files('redraw_pixels_all_on', redraw_pixels_all_on)
    rows_off = [[0] * 128] * 64
    redraw_pixels_all_off = blipper.RedrawPixels.from_matrix(display, rows_off)
    dump_to_files('redraw_pixels_all_off', redraw_pixels_all_off)
    rows_random = [[0] * 128] * 64
    for row_id, row in enumerate(rows_random):
        for col_id, col in enumerate(row):
            rows_random[row_id][col_id] = random.randint(0, 1)
    redraw_pixels_all_random = blipper.RedrawPixels.from_matrix(
            display, rows_random)
    dump_to_files('redraw_pixels_all_random', redraw_pixels_all_random)


if __name__ == '__main__':
    dump()

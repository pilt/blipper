import random

import blipper

def dump():
    insufficient_funds = blipper.InsufficientFunds.from_balance(1337)
    with open('dumps/insufficient_funds.bin', 'wb') as f:
        f.write(bytes(insufficient_funds))

    new_balance = blipper.NewBalance.from_new_balance(1337)
    with open('dumps/new_balance.bin', 'wb') as f:
        f.write(bytes(new_balance))

    with open('dumps/ping.bin', 'wb') as f:
        f.write(bytes(blipper.Ping()))

    with open('dumps/pong.bin', 'wb') as f:
        f.write(bytes(blipper.Pong()))

    with open('dumps/ok.bin', 'wb') as f:
        f.write(bytes(blipper.Ok()))

    with open('dumps/fail.bin', 'wb') as f:
        f.write(bytes(blipper.Fail()))

    display = 0
    rows_on = [[1] * 128] * 64
    redraw_pixels_all_on = blipper.RedrawPixels.from_matrix(display, rows_on)
    with open('dumps/redraw_pixels_all_on.bin', 'wb') as f:
        f.write(bytes(redraw_pixels_all_on))
    rows_off = [[0] * 128] * 64
    redraw_pixels_all_off = blipper.RedrawPixels.from_matrix(display, rows_off)
    with open('dumps/redraw_pixels_all_off.bin', 'wb') as f:
        f.write(bytes(redraw_pixels_all_off))
    rows_random = [[0] * 128] * 64
    for row_id, row in enumerate(rows_random):
        for col_id, col in enumerate(row):
            rows_random[row_id][col_id] = random.randint(0, 1)
    redraw_pixels_all_random = blipper.RedrawPixels.from_matrix(display, 
                                                                rows_random)
    with open('dumps/redraw_pixels_all_random.bin', 'wb') as f:
        f.write(bytes(redraw_pixels_all_random))


if __name__ == '__main__':
    dump()

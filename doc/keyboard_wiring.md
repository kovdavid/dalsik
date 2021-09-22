# Keyboard wiring

Each key must be somehow wired to the MCU, so it can determine if a key is pressed or released.

A key is just an electric switch; by pressing a key create a connection between two pieces of metal.

<img src="electric_switch.jpg" alt="switch" style="zoom:50%;" />

The Let's Split keyboard has 24 switches on each side. If we were to wire up each key individually, we would need 24*2=48 pins (+ and - cable), which the Pro Micro simply does not have. We can optimize this by using a common group cable. This way we would need 24+1=25 pins, which is still to many.

Because of the limited pin numbers of these small microcontrollers, the keys are wired up in a [matrix](http://pcbheaven.com/wikipages/How_Key_Matrices_Works/):

![keyboard_matrix](keyboard_matrix.png)

Each row and each column is connected to 1 pin. This way we need 4 (rows) + 6 (columns) = 10 pins for our keyboard to read every single key. Each switch has two legs. The left legs of the switches are interconnected in each row, while the right legs are connected in each column (or vice-versa. It does not really matter).

The way this key matrix is scanned is documented in greater detail [here](matrix_scan.md).
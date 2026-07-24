import socket
import struct
import time

# 18-byte binary layout matching struct
PACKET_FORMAT = '=c Q I I c'

def send_order(sock, msg_type, order_id, price, qty, side):
    packet = struct.pack(PACKET_FORMAT, msg_type, order_id, price, qty, side)
    sock.sendall(packet)
    print(f"Sent: {msg_type.decode()} order_id={order_id}")

def main():
    host = '127.0.0.1'
    port = 8080

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print(f"Connecting to {host}:{port}...")
        s.connect((host, port))

        send_order(s, b'A', 1, 15000, 100, b'B')
        time.sleep(0.5)

        send_order(s, b'A', 2, 15100, 50, b'S')
        time.sleep(0.5)

        send_order(s, b'C', 1, 0, 0, b'B')

        print("Test sequence complete.")

if __name__ == "__main__":
    main()
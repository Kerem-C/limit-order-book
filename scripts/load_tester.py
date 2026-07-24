import socket
import struct
import time
import sys

# 18-byte binary layout matching struct
PACKET_FORMAT = '=c Q I I c'
NUM_ORDERS = 1_000_000

def main():
    host = '127.0.0.1'
    port = 8080

    print(f"Pre-packing {NUM_ORDERS:,} orders into memory...")

    single_packet = struct.pack(PACKET_FORMAT, b'A', 1, 15000, 100, b'B')
    payload = single_packet * NUM_ORDERS # ~17 MB of orders
    payload_size_mb = len(payload) / (1024 * 1024)
    
    print(f"Payload ready: {payload_size_mb:.2f} MB.")
    input("Press Enter to blast the server...")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((host, port))
        except ConnectionRefusedError:
            print(f"Failed to connect. Is the C++ server running on port {port}?")
            sys.exit(1)
            
        print(f"Connected. Blasting data to the TCP socket...")
        start_time = time.perf_counter()
        
        s.sendall(payload)
        
        end_time = time.perf_counter()
        
        elapsed = end_time - start_time
        mps = NUM_ORDERS / elapsed
        
        print(f"--- Client-Side Metrics ---")
        print(f"Total time to push to OS: {elapsed:.4f} seconds")
        print(f"Theoretical throughput:   {mps:,.0f} messages/sec")

if __name__ == "__main__":
    main()
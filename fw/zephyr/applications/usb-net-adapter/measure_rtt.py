import socket
import time
import argparse

# Default values
DEFAULT_MESSAGE = "Test message"
UDP_IP = "192.0.2.1"
UDP_PORT = 5001
TCP_PORT = 5002

def measure_udp_rtt(message):
    """Measure Round-Trip Time for UDP."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(1)  # Timeout for response
    start_time = time.time()  # Record start time
    sock.sendto(message.encode(), (UDP_IP, UDP_PORT))  # Send message

    try:
        data, addr = sock.recvfrom(1024)  # Wait for response
        end_time = time.time()  # Record end time
        rtt = (end_time - start_time) * 1000  # Convert to milliseconds
        print(f"Response: {data.decode()}")
        print(f"Round Trip Time: {rtt:.2f} ms")
    except socket.timeout:
        print("No response received (timeout)")
    finally:
        sock.close()

def measure_tcp_rtt(message):
    """Measure Round-Trip Time for TCP."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(1)  # Timeout for response

    try:
        sock.connect((UDP_IP, TCP_PORT))  # Connect to the server
        start_time = time.time()  # Record start time
        sock.sendall(message.encode())  # Send message

        data = sock.recv(1024)  # Wait for response
        end_time = time.time()  # Record end time
        rtt = (end_time - start_time) * 1000  # Convert to milliseconds
        print(f"Response: {data.decode()}")
        print(f"Round Trip Time: {rtt:.2f} ms")
    except socket.timeout:
        print("No response received (timeout)")
    except ConnectionRefusedError:
        print("Connection refused. Ensure the server is running.")
    finally:
        sock.close()

def main():
    parser = argparse.ArgumentParser(description="Measure RTT for UDP or TCP packets.")
    parser.add_argument("-u", action="store_true", help="Use UDP for communication")
    parser.add_argument("-t", action="store_true", help="Use TCP for communication")
    parser.add_argument("-m", type=str, help="Message to send")
    
    args = parser.parse_args()

    if not args.u and not args.t:
        print("Please specify either -u (UDP) or -t (TCP).")
        return

    message = args.m if args.m else DEFAULT_MESSAGE

    if args.u:
        print("Measuring RTT using UDP...")
        measure_udp_rtt(message)
    elif args.t:
        print("Measuring RTT using TCP...")
        measure_tcp_rtt(message)

if __name__ == "__main__":
    main()

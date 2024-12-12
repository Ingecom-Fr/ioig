import socket
import time
import argparse

# Default message value
DEFAULT_MESSAGE = (
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
    "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
    "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. "
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()_+-=[]{}|;:'\",.<>?/\\"    
)

# message size,  max=512
MSG_SIZE = 64

# Ensure the message is exactly MSG_SIZE bytes
if len(DEFAULT_MESSAGE) > MSG_SIZE:
    DEFAULT_MESSAGE = DEFAULT_MESSAGE[:MSG_SIZE]
elif len(DEFAULT_MESSAGE) < MSG_SIZE:
    DEFAULT_MESSAGE += " " * (MSG_SIZE - len(DEFAULT_MESSAGE))  

DEV_IP = "192.0.2.1"
UDP_PORT = 5001
TCP_PORT = 5002

def measure_udp_rtt(message):
    """Measure Round-Trip Time for UDP using time_ns()."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(1)  # Timeout for response
    try:
        start_time = time.time_ns()  # Record start time in nanoseconds
        sock.sendto(message.encode(), (DEV_IP, UDP_PORT))  # Send message
        data, addr = sock.recvfrom(1024)  # Wait for response
        end_time = time.time_ns()  # Record end time in nanoseconds
        rtt_us = (end_time - start_time) / 1000  # Convert to microseconds
        print(f"Response: {data.decode()}")
        print(f"Round Trip Time: {rtt_us:.2f} µs")
    except socket.timeout:
        print("No response received (timeout)")
    finally:
        sock.close()

def measure_tcp_rtt(message):
    """Measure Round-Trip Time for TCP using time_ns()."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(1)  # Timeout for response

    try:
        sock.connect((DEV_IP, TCP_PORT))  # Connect to the server
        start_time = time.time_ns()  # Record start time in nanoseconds
        sock.sendall(message.encode())  # Send message

        data = sock.recv(1024)  # Wait for response
        end_time = time.time_ns()  # Record end time in nanoseconds
        rtt_us = (end_time - start_time) / 1000  # Convert to microseconds
        print(f"Response: {data.decode()}")
        print(f"Round Trip Time: {rtt_us:.2f} µs")
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
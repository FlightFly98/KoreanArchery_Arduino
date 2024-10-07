import socket
import subprocess
import time
from ipaddress import ip_network, ip_address

# ESP8266의 AP 서브넷
ESP_SUBNET = ip_network("192.168.4.0/24")
CONNECT_MESSAGE = b"Hello from PC"
CHECK_MESSAGE = b"Check connection"
ACK_MESSAGE = b"ACK"

# PowerShell 스크립트를 실행하여 컴퓨터의 Wi-Fi IP 주소를 얻음
def get_ip_address():
    result = subprocess.run(["powershell.exe", "-ExecutionPolicy", "Bypass", "-File", "..\\Server\\GetIP.ps1"], capture_output=True, text=True)
    ip_address = result.stdout.strip()
    print(f"get_ip_address() returned: {ip_address}")
    return ip_address if ip_address else None

# PC가 Gukgung_Wifi AP에 연결되었는지 확인
def is_connected_to_esp_ap(pc_ip):
    try:
        return ip_address(pc_ip) in ESP_SUBNET
    except ValueError:
        return False

# UDP 메시지를 아두이노 장치에 전송하고 응답을 기다림
def send_udp_message_and_wait(ip_address, port, message, ack_message):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(3)  # 3초 타임아웃
    for _ in range(3):  # 최대 3번 시도
        print(f"Sending message to {ip_address}:{port}")
        sock.sendto(message, (ip_address, port))
        try:
            data, addr = sock.recvfrom(1024)  # 1024 바이트 버퍼 크기
            if data == ack_message:
                print(f"Received ACK from {ip_address}:{port}")
                return True
        except socket.timeout:
            print(f"No response from {ip_address}:{port}, retrying...")
    sock.close()
    return False

# 아두이노 장치의 연결 상태를 확인하고 필요시 재연결 시도
def check_arduino_connections(arduino_devices):
    for device in arduino_devices:
        ip = device["ip"]
        port = device["port"]
            
        if send_udp_message_and_wait(ip, port, CHECK_MESSAGE, ACK_MESSAGE):
            print(f"Device at {ip}:{port} is still connected.")
            device["acknowledged"] = True
        else:
            print(f"Device at {ip}:{port} did not respond. Trying to reconnect...")
            if send_udp_message_and_wait(ip, port, CONNECT_MESSAGE, ACK_MESSAGE):
                print(f"Device at {ip}:{port} reconnected.")
                device["acknowledged"] = True
            else:
                print(f"Failed to reconnect device at {ip}:{port}.")
                device["acknowledged"] = False

# 메인 함수
def main():
    print("Checking connection to Gukgung_Wifi AP...")
    pc_ip = None
    while True:
        pc_ip = get_ip_address()
        if pc_ip:
            print(f"Obtained PC IP: {pc_ip}")
            # IP 주소의 타입 확인
            print(f"Type of pc_ip: {type(pc_ip)}")
            if is_connected_to_esp_ap(pc_ip):
                print("Connected to Gukgung_Wifi.")
                print(f"PC IP Address: {pc_ip}")
                break
            else:
                print("Not connected to Gukgung_Wifi AP. Retrying in 5 seconds...")
        else:
            print("Failed to obtain PC IP. Retrying in 5 seconds...")
        time.sleep(5)

    # 아두이노 장치의 IP 주소 및 포트 리스트
    arduino_devices = [
        # {"ip": "192.168.4.1", "port": 12345, "acknowledged": False}, # 낙전 감지 센서
        {"ip": "192.168.4.2", "port": 12346, "acknowledged": False}, # 줌손, AP
        {"ip": "192.168.4.3", "port": 12347, "acknowledged": False}, # 깍지손
        {"ip": "192.168.4.4", "port": 12348, "acknowledged": False}, # 충격 감지 센서
        # {"ip": "192.168.4.5", "port": 12350, "acknowledged": False}, 
    ]

    # 첫 번째 연결 확인 메시지 전송
    check_arduino_connections(arduino_devices)

    # 주기적으로 연결 상태 확인
    while True:
        print("Checking connections...")
        check_arduino_connections(arduino_devices)
        time.sleep(5)  # 15초마다 연결 상태 확인

if __name__ == "__main__":
    main()

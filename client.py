import socket

# Create a socket to connect to the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the server at localhost and port 1337
s.connect(('localhost', 1337))

# Send a message to the server
s.sendall(b"Hello, server!\n")

# Receive the server's response
response = s.recv(1024)
print("Server response:", response.decode())

# Close the connection
s.close()

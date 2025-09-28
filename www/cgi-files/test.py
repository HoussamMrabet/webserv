#!/usr/bin/env python3

import os
import sys
import html
from urllib.parse import parse_qs
import traceback

def enable_debugging():
    """Simple debugging function to replace cgitb"""
    def handle_exception(exc_type, exc_value, exc_traceback):
        print("Content-Type: text/html\n")
        print("<h1>CGI Error</h1>")
        print("<pre>")
        traceback.print_exception(exc_type, exc_value, exc_traceback)
        print("</pre>")
    
    sys.excepthook = handle_exception

def parse_form_data():
    """Parse POST form data without using cgi module"""
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        return parse_qs(post_data)
    return {}

# Enable debugging
enable_debugging()

print("Content-Type: text/html\n")  # HTTP header

# Parse POST data
form_data = parse_form_data()

# Retrieve data from the POST request (parse_qs returns lists)
name = form_data.get("name", ["No Name Provided"])[0]
email = form_data.get("email", ["No Email Provided"])[0]

# Escape HTML to prevent XSS (important security improvement!)
name = html.escape(name)
email = html.escape(email)

# Generate a response
print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>CGI POST Test</title>
</head>
<body>
    <h1>CGI POST Test</h1>
    <p><strong>Name:</strong> {name}</p>
    <p><strong>Email:</strong> {email}</p>
</body>
</html>
""")

# # test using 
# curl -v -X POST -d "name=John Doe&email=johndoe@example.com" http://localhost:8080/cgi-files/test.py
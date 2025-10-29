#!/usr/bin/env python3

import os
import sys
import html
from urllib.parse import parse_qs

try:
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length) if content_length > 0 else ""
    form = parse_qs(post_data)
except Exception:
    form = {}

name = html.escape(form.get("name", ["No Name Provided"])[0])
email = html.escape(form.get("email", ["No Email Provided"])[0])

print("Content-Type: text/html\n")
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI POST Test</title>
</head>
<body>
    <h1>CGI POST Test</h1>
    <p><strong>Name:</strong> {name}</p>
    <p><strong>Email:</strong> {email}</p>
</body>
</html>""")


# # test using 
# curl -d "name=sunny&email=sunny@test.com" http://localhost:8080/cgi-bin/test.py
# curl -v -X POST -d "name=sunny&email=sunny@test.com" http://localhost:8080/cgi-bin/test.py

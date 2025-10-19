#!/usr/bin/env python3
import cgi
import cgitb
import datetime
import os

cgitb.enable()

# Parse form data
form = cgi.FieldStorage()
name = form.getvalue("name", "Guest")

# Start with proper headers
print("Content-Type: text/html")
print()

# Begin HTML output
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Long CGI Output Test</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; }")
print("table { border-collapse: collapse; width: 100%; }")
print("td, th { border: 1px solid #ccc; padding: 8px; text-align: left; }")
print("</style>")
print("</head>")
print("<body>")

print(f"<h1>Hello, {name}!</h1>")
print(f"<p>Current server time is: {datetime.datetime.now()}</p>")
print("<p>Environment Variables:</p>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}</strong>: {value}</li>")
print("</ul>")

print("<hr>")
print("<h2>Test Table with 100 Rows</h2>")
print("<table>")
print("<tr><th>#</th><th>Data</th><th>More Data</th></tr>")
for i in range(1, 101):
    print(f"<tr><td>{i}</td><td>Row {i} data</td><td>Extra info for row {i}</td></tr>")
print("</table>")

print("<hr>")
print("<h2>Dummy Text Section</h2>")
lorem = (
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
    "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
    "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
)

for i in range(100):
    print(f"<p>{lorem}</p>")

print("end.")
print("</body>")
print("</html>")

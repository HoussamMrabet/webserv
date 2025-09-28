import cgi
import cgitb

cgitb.enable()

name = "test"

form = cgi.FieldStorage()
name = form.getvalue("name")

print("<html>")
print("<body>")
print("<h1>Hello, {}!</h1>".format(name))
print("</body>")
print("</html>")

# test using:
# curl "localhost:8080/cgi-files/file.py?name=sunny"

# or this:
# curl -X POST -d "name=sunny" http://localhost:8080/cgi-files/file.py